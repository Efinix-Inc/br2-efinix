// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 * Copyright (C) 2026 Efinix Inc. All rights reserved.
 */

#include <linux/io.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_reserved_mem.h>
#include <linux/platform_device.h>
#include <linux/remoteproc.h>
#include <linux/hrtimer.h>
#include <linux/cpumask.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <asm/sbi.h>

#include "remoteproc_internal.h"

/* Must match common/shared_layout.h in the firmware. */
#define SHM_RSC_OFF             0x0000
#define SHM_RSC_SIZE            0x0100
#define SHM_KICK_R2L_OFF        0x0100
#define SHM_KICK_L2R_OFF        0x0104
#define SHM_REMOTE_READY_OFF    0x0110
#define SHM_LINUX_READY_OFF     0x0114
#define SHM_LINUX_CTRL_OFF      0x0118
#define SHM_VRING0_OFF          0x2000
#define SHM_VRING1_OFF          0x4000
#define SHM_VRING_SIZE          0x2000

/* Magic values in the ready flags. Both sides agree on these. */
#define REMOTE_READY_MAGIC      0x524D4554  /* 'RMET' */
#define LINUX_READY_MAGIC       0x4C4E5852  /* 'LNXR' */

#define REMOTE_READY_TIMEOUT_MS 5000
#define CTRL_MSG_NONE        0x00000000U
#define RSC_TABLE_BYTES      88

static unsigned int rproc_poll_us = 1000;
module_param(rproc_poll_us, uint, 0644);
MODULE_PARM_DESC(rproc_poll_us,
		 "Microsecond tick for polling inbound kick");

static bool rproc_use_ipi = true;
module_param(rproc_use_ipi, bool, 0644);
MODULE_PARM_DESC(rproc_use_ipi,
		 "Catch inbound kick via the SSIP doorbell hook");

extern void riscv_amp_set_ipi_hook(void (*fn)(void *cookie), void *cookie);

struct efinix_rproc {
	struct rproc		*rproc;
	struct device		*dev;
	u32			remote_hartid;

	phys_addr_t		fw_pa;
	void __iomem		*fw_va;
	size_t			fw_size;

	phys_addr_t		shm_pa;
	void __iomem		*shm_va;
	size_t			shm_size;
	bool			hart_started;
	void __iomem		*rsc_va;
	void __iomem		*kick_r2l_va;
	void __iomem		*kick_l2r_va;
	void __iomem		*remote_ready_va;
	void __iomem		*linux_ready_va;
	void __iomem		*linux_ctrl_msg_va;

	struct hrtimer		poll_timer;
	bool			poll_running;
	struct work_struct	notify_work;
	u32			last_r2l_seen;

	bool			ipi_active;
	atomic_t		ipi_doorbells;
	bool			inbound_logged;
};

static int efinix_rproc_hart_status(unsigned long hartid)
{
	struct sbiret r = sbi_ecall(SBI_EXT_HSM,
				    SBI_EXT_HSM_HART_STATUS,
				    hartid, 0, 0, 0, 0, 0);
	if (r.error)
		return sbi_err_map_linux_errno(r.error);
	return (int)r.value;
}

static int efinix_rproc_hart_start(unsigned long hartid, unsigned long start_addr)
{
	int state = efinix_rproc_hart_status(hartid);
	struct sbiret r;

	if (state == SBI_HSM_STATE_STARTED) {
		pr_debug("hart %lu already STARTED, skipping hart_start\n",
			 hartid);
		return 0;
	}

	r = sbi_ecall(SBI_EXT_HSM, SBI_EXT_HSM_HART_START,
		      hartid, start_addr, 0 /* opaque */, 0, 0, 0);
	if (r.error) {
		pr_err("sbi_hsm_hart_start(%lu, %#lx) failed: %ld\n",
		       hartid, start_addr, r.error);
		return sbi_err_map_linux_errno(r.error);
	}
	return 0;
}

static void efinix_rproc_send_ipi(unsigned long hartid)
{
	unsigned long hart_mask = 1UL << (hartid % BITS_PER_LONG);
	unsigned long hart_mask_base = hartid - (hartid % BITS_PER_LONG);

	sbi_ecall(SBI_EXT_IPI, SBI_EXT_IPI_SEND_IPI,
		  hart_mask, hart_mask_base, 0, 0, 0, 0);
}

static void efinix_rproc_notify_work_fn(struct work_struct *w)
{
	struct efinix_rproc *priv = container_of(w, struct efinix_rproc, notify_work);

	rproc_vq_interrupt(priv->rproc, 0);
	rproc_vq_interrupt(priv->rproc, 1);
}

static void efinix_rproc_ipi_doorbell(void *cookie)
{
	struct efinix_rproc *priv = cookie;
	u32 now;

	if (!READ_ONCE(priv->ipi_active))
		return;

	now = readl_relaxed(priv->kick_r2l_va);
	if (now != READ_ONCE(priv->last_r2l_seen)) {
		WRITE_ONCE(priv->last_r2l_seen, now);
		atomic_inc(&priv->ipi_doorbells);
		schedule_work(&priv->notify_work);
	}
}

static enum hrtimer_restart efinix_rproc_poll_fn(struct hrtimer *t)
{
	struct efinix_rproc *priv = container_of(t, struct efinix_rproc, poll_timer);
	u32 now;

	now = readl_relaxed(priv->kick_r2l_va);
	if (now != priv->last_r2l_seen) {
		priv->last_r2l_seen = now;
		schedule_work(&priv->notify_work);
	}

	hrtimer_forward_now(t, ns_to_ktime((u64)rproc_poll_us * NSEC_PER_USEC));
	return HRTIMER_RESTART;
}

static void efinix_rproc_inbound_start(struct efinix_rproc *priv)
{
	priv->last_r2l_seen = readl_relaxed(priv->kick_r2l_va);
	WRITE_ONCE(priv->inbound_logged, false);

	if (rproc_use_ipi) {
		atomic_set(&priv->ipi_doorbells, 0);
		smp_wmb();
		WRITE_ONCE(priv->ipi_active, true);
		return;
	}

	if (priv->poll_running)
		return;

	hrtimer_init(&priv->poll_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	priv->poll_timer.function = efinix_rproc_poll_fn;
	hrtimer_start(&priv->poll_timer,
		      ns_to_ktime((u64)rproc_poll_us * 1000),
		      HRTIMER_MODE_REL);

	priv->poll_running = true;
}

static void efinix_rproc_inbound_stop(struct efinix_rproc *priv)
{
	if (rproc_use_ipi) {
		WRITE_ONCE(priv->ipi_active, false);
		cancel_work_sync(&priv->notify_work);
		return;
	}

	if (!priv->poll_running)
		return;

	hrtimer_cancel(&priv->poll_timer);
	cancel_work_sync(&priv->notify_work);
	priv->poll_running = false;
}

static int efinix_rproc_prepare(struct rproc *rproc)
{
	struct efinix_rproc *priv = rproc->priv;
	struct rproc_mem_entry *m0, *m1;

	m0 = rproc_mem_entry_init(priv->dev,
				  (void __force *)priv->shm_va + SHM_VRING0_OFF,
				  priv->shm_pa + SHM_VRING0_OFF,
				  SHM_VRING_SIZE,
				  priv->shm_pa + SHM_VRING0_OFF,
				  NULL,  /* alloc */
				  NULL,  /* release */
				  "vdev0vring0");
	if (!m0)
		return -ENOMEM;
	rproc_add_carveout(rproc, m0);

	m1 = rproc_mem_entry_init(priv->dev,
				  (void __force *)priv->shm_va + SHM_VRING1_OFF,
				  priv->shm_pa + SHM_VRING1_OFF,
				  SHM_VRING_SIZE,
				  priv->shm_pa + SHM_VRING1_OFF,
				  NULL,
				  NULL,
				  "vdev0vring1");
	if (!m1)
		return -ENOMEM;
	rproc_add_carveout(rproc, m1);

	dev_info(priv->dev,
		 "registered vring carveouts at %pa (offsets %#x, %#x)\n",
		 &priv->shm_pa, SHM_VRING0_OFF, SHM_VRING1_OFF);

	return 0;
}

static int efinix_rproc_start(struct rproc *rproc)
{
	struct efinix_rproc *priv = rproc->priv;
	int ret;

	efinix_rproc_inbound_start(priv);

	if (priv->hart_started)
		return 0;

	ret = efinix_rproc_hart_start(priv->remote_hartid, rproc->bootaddr);
	if (ret) {
		dev_err(priv->dev, "sbi_hsm_hart_start failed: %d\n", ret);
		efinix_rproc_inbound_stop(priv);
		return ret;
	}

	priv->hart_started = true;
	return 0;
}

static int efinix_rproc_parse_fw_shm(struct rproc *rproc, const struct firmware *fw)
{
	struct efinix_rproc *priv = rproc->priv;
	void *table;
	u32 magic;
	int ret;

	ret = rproc_elf_load_segments(rproc, fw);
	if (ret) {
		dev_err(priv->dev, "elf load_segments failed: %d\n", ret);
		return ret;
	}

	writel(0, priv->remote_ready_va);
	writel(0, priv->kick_r2l_va);
	writel(0, priv->kick_l2r_va);
	writel(CTRL_MSG_NONE, priv->linux_ctrl_msg_va);
	wmb();

	ret = efinix_rproc_hart_start(priv->remote_hartid, rproc->bootaddr);
	if (ret) {
		dev_err(priv->dev, "sbi_hsm_hart_start failed: %d\n", ret);
		return ret;
	}

	priv->hart_started = true;

	ret = readx_poll_timeout(readl, priv->remote_ready_va, magic,
				 magic == REMOTE_READY_MAGIC,
				 200, 2 * USEC_PER_SEC);
	if (ret) {
		dev_err(priv->dev,"remote did not publish resource table \n");
		return -ETIMEDOUT;
	}

	table = kzalloc(RSC_TABLE_BYTES, GFP_KERNEL);
	if (!table)
		return -ENOMEM;

	memcpy_fromio(table, priv->rsc_va, RSC_TABLE_BYTES);

	rproc->cached_table = table;
	rproc->table_sz = RSC_TABLE_BYTES;
	rproc->table_ptr = rproc->cached_table;

	return 0;
}

static int efinix_rproc_load_noop(struct rproc *rproc, const struct firmware *fw)
{
	return 0;
}

static struct resource_table *
efinix_rproc_get_loaded_rsc_table(struct rproc *rproc, const struct firmware *fw)
{
	struct efinix_rproc *priv = rproc->priv;

	return (struct resource_table __force *)priv->rsc_va;
}

static int efinix_rproc_stop(struct rproc *rproc)
{
	struct efinix_rproc *priv = rproc->priv;
	int state = -1;
	int ret = -1;
	int i;

	/* Todo: use macro for 0xFF00FF00 */
	writel(0xFF00FF00, priv->linux_ctrl_msg_va);
	wmb();
	efinix_rproc_send_ipi(priv->remote_hartid);

	for (i = 0; i < 200; i++) {
		state = efinix_rproc_hart_status(priv->remote_hartid);
		if (state == SBI_HSM_STATE_STOPPED) {
			ret = 0;
			break;
		}
		if ((i % 20) == 19)
			efinix_rproc_send_ipi(priv->remote_hartid);
		usleep_range(3000, 5000);
	}

	efinix_rproc_inbound_stop(priv);
	priv->hart_started = false;

	writel(CTRL_MSG_NONE, priv->linux_ctrl_msg_va);
	wmb();

	return ret;
}

static void efinix_rproc_kick(struct rproc *rproc, int vqid)
{
	struct efinix_rproc *priv = rproc->priv;
	u32 next;

	next = readl_relaxed(priv->kick_l2r_va) + 1;
	writel_relaxed(next, priv->kick_l2r_va);

	wmb();
	efinix_rproc_send_ipi(priv->remote_hartid);
}

static void *efinix_rproc_da_to_va(struct rproc *rproc, u64 da, size_t len, bool *is_iomem)
{
	struct efinix_rproc *priv = rproc->priv;

	if (da >= priv->fw_pa && da + len <= priv->fw_pa + priv->fw_size) {
		if (is_iomem)
			*is_iomem = true;
		return (void __force *)priv->fw_va + (da - priv->fw_pa);
	}

	dev_warn(priv->dev, "da_to_va: out-of-range request da = 0x%llx len = %d\n",
		 da, len);
	return NULL;
}

static const struct rproc_ops efinix_rproc_ops = {
	.prepare                = efinix_rproc_prepare,
	.start                  = efinix_rproc_start,
	.stop                   = efinix_rproc_stop,
	.kick                   = efinix_rproc_kick,
	.da_to_va               = efinix_rproc_da_to_va,
	.parse_fw               = efinix_rproc_parse_fw_shm,
	.load                   = efinix_rproc_load_noop,
	.find_loaded_rsc_table  = efinix_rproc_get_loaded_rsc_table,
	.sanity_check           = rproc_elf_sanity_check,
	.get_boot_addr          = rproc_elf_get_boot_addr,
};

static int efinix_rproc_map(struct device *dev, const char *propname,
			  phys_addr_t *pa, void __iomem **va, size_t *size)
{
	struct device_node *node;
	struct resource res;
	int ret;

	node = of_parse_phandle(dev->of_node, propname, 0);
	if (!node) {
		dev_err(dev, "no %s phandle in DT\n", propname);
		return -ENODEV;
	}

	ret = of_address_to_resource(node, 0, &res);
	of_node_put(node);
	if (ret) {
		dev_err(dev, "%s: of_address_to_resource failed: %d\n",
			propname, ret);
		return ret;
	}

	*pa = res.start;
	*size = resource_size(&res);

	*va = ioremap_wc(*pa, *size);
	if (!*va) {
		dev_err(dev, "%s: ioremap_wc %pa+%#zx failed\n",
			propname, pa, *size);
		return -ENOMEM;
	}

	return 0;
}

static int efinix_rproc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct efinix_rproc *priv;
	struct rproc *rproc;
	const char *fw_name = "openampEcho.elf";
	int ret;

	rproc = rproc_alloc(dev, dev_name(dev), &efinix_rproc_ops,
			    fw_name /* NULL */, sizeof(*priv));
	if (!rproc)
		return -ENOMEM;

	priv = rproc->priv;
	priv->rproc = rproc;
	priv->dev = dev;

	rproc->auto_boot = false;

	ret = of_property_read_u32(np, "remote-hartid", &priv->remote_hartid);
	if (ret) {
		dev_err(dev, "missing remote-hartid in DT\n");
		goto err_put;
	}

	ret = efinix_rproc_map(dev, "fw-memory-region",
			     &priv->fw_pa, &priv->fw_va, &priv->fw_size);
	if (ret)
		goto err_put;

	ret = efinix_rproc_map(dev, "shm-memory-region",
			     &priv->shm_pa, &priv->shm_va, &priv->shm_size);
	if (ret)
		goto err_unmap_fw;

	priv->rsc_va          = priv->shm_va + SHM_RSC_OFF;
	priv->kick_r2l_va     = priv->shm_va + SHM_KICK_R2L_OFF;
	priv->kick_l2r_va     = priv->shm_va + SHM_KICK_L2R_OFF;
	priv->remote_ready_va = priv->shm_va + SHM_REMOTE_READY_OFF;
	priv->linux_ready_va  = priv->shm_va + SHM_LINUX_READY_OFF;
	priv->linux_ctrl_msg_va     = priv->shm_va + SHM_LINUX_CTRL_OFF;

	writel_relaxed(0, priv->kick_l2r_va);
	writel_relaxed(0, priv->kick_r2l_va);

	INIT_WORK(&priv->notify_work, efinix_rproc_notify_work_fn);
	if (rproc_use_ipi) {
		atomic_set(&priv->ipi_doorbells, 0);
		riscv_amp_set_ipi_hook(efinix_rproc_ipi_doorbell, priv);
	}

	platform_set_drvdata(pdev, rproc);

	ret = rproc_add(rproc);
	if (ret) {
		dev_err(dev, "rproc_add failed: %d\n", ret);
		goto err_unmap_shm;
	}

	return 0;

err_unmap_shm:
	if (rproc_use_ipi)
		riscv_amp_set_ipi_hook(NULL, NULL);
	iounmap(priv->shm_va);
err_unmap_fw:
	iounmap(priv->fw_va);
err_put:
	rproc_free(rproc);
	return ret;
}

static int efinix_rproc_remove(struct platform_device *pdev)
{
	struct rproc *rproc = platform_get_drvdata(pdev);
	struct efinix_rproc *priv = rproc->priv;

	rproc_del(rproc);

	efinix_rproc_inbound_stop(priv);
	if (rproc_use_ipi)
		riscv_amp_set_ipi_hook(NULL, NULL);

	iounmap(priv->shm_va);
	iounmap(priv->fw_va);
	rproc_free(rproc);
	return 0;
}

static const struct of_device_id efinix_rproc_of_match[] = {
	{ .compatible = "efinix,ti375_rproc" },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, efinix_rproc_of_match);

static struct platform_driver efinix_rproc_driver = {
	.probe  = efinix_rproc_probe,
	.remove = efinix_rproc_remove,
	.driver = {
		.name           = "efinix_ti375_rproc",
		.of_match_table = efinix_rproc_of_match,
	},
};

module_platform_driver(efinix_rproc_driver);

MODULE_AUTHOR("sia jee heng <jhsia@efinixinc.com>");
MODULE_DESCRIPTION("Efinix TI375 RemoteProc");
MODULE_LICENSE("GPL");
