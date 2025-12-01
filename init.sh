#!/bin/bash -e

# args1: board
# args2: path/to/soc.h

OPTIND=1
unset RECONFIGURE
unset RECONFIGURE_ALL
unset OPT_DIR
unset EXAMPLE_DESIGN
unset HARDEN_SOC
unset UNIFIED_HW
unset HW_FEATURES
unset EXTRA_HW_FEATURES
unset X11_GRAPHICS
unset MACHINE_ARCH
unset USE_EMMC_BOOT

BOARD=$1
SOC_H=$2

HW_FEATURES=""
MACHINE_ARCH="32"
INPUT_FILE="VERSION"
substr=""

PROJ_DIR=$PWD
BR2_EXTERNAL_DIR=$PROJ_DIR
BR2_DEFCONFIG_DIR="${BR2_EXTERNAL_DIR}/configs"
BR2_DEFCONFIG=""

EFINIX_DIR="$BR2_EXTERNAL_DIR/boards/efinix"
COMMON_DIR="$EFINIX_DIR/common"
DT_DIR="$COMMON_DIR/sapphire-soc-dt-generator"
JSON_FILE="$DT_DIR/config/default.json"

# Text colors
WHITE='\033[0;37m'
BLACK='\033[0;30m'
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'

# Background colors
BG_WHITE='\033[47m'
BG_BLACK='\033[40m'
BG_RED='\033[41m'
BG_GREEN='\033[42m'
BG_YELLOW='\033[43m'
BG_BLUE='\033[44m'
NC='\033[0m'	# No color

# Get the terminal width
COLUMNS=$(tput cols)

function title()
{
	# Print combined text and background colors
	echo -e "\n${BLACK}${BG_WHITE}>>> $1 ${NC}"
}

function pr_err()
{
	echo -e "${RED}ERROR${NC}: $1"
}

function pr_info()
{
	echo -e "${GREEN}INFO${NC}: $1"
}

function self_check()
{
	if [ $2 -eq 0 ]; then
		echo -e "Check: $1 ${RED}NO${NC}"
	else
		echo -e "Check: $1 ${GREEN}YES${NC}"
	fi
}

function usage()
{
	echo "Usage: init.sh [board] [path/to/soc.h] [-c][-d][-m][-r][-a][-e][-u][-s][-x][-w]"
	echo
	echo "Positional arguments:"
	echo "	board			Development kit name such as t120f324, ti60f225, ti180j484, ti375c529"
	echo "	soc			Path to 'soc.h'. This file is located in Efinity project directory."
	echo "				For example,"
	echo "				my_project/embedded_sw/SapphireCore/bsp/efinix/EfxSapphireSoc/include/soc.h"
	echo "				my_project/embedded_sw/efx_hard_soc/bsp/efinix/EfxSapphireSoc/include/soc.h"
	echo
	echo "Optional arguments:"
	echo "	-h			Show this help message and exit"
	echo "	-c			Reset the repo to the original state"
	echo "	-d			Rename default build directory name"
	echo "				By default is <board>_build"
	echo "				Example, if board is ti60f225 then, the name of build directory is 'ti60f225_build'."
	echo "	-m			Machine architecture type either 32 or 64 bits. Default is 32."
	echo "	-r			Reconfigure the Buildroot configuration. This option will not regenerate device tree."
	echo "	-a			Reconfigure the Buildroot configuration and regenerate Linux device tree."
	echo "	-e                      Generate Linux devcie tree for SoC example design."
	echo "	-u			Generate Linux device tree for unified hardware design for Ti180J484 and Ti375C529"
	echo "	-s			Set hardware features to enable in the Linux kernel. Must be in comma seperated."
	echo "				For example, spi,i2c,gpio,ethernet,dma,framebuffer"
	echo "	-x			Enable X11 graphics for unified hardware design. This enable framebuffer, DMA and USB drivers."
	echo "				Not compatible with camera (evsoc driver). This optional argument requires -u to be set first."
	echo "	-w			Use eMMC u-boot configuration for eMMC storage support"
	echo
	echo "Example usage,"
	echo "$	source init.sh t120f324 ~/efinity/2022.1/project/soc/ip/soc1/T120F324_devkit/embedded_sw/soc1/bsp/efinix/EfxSapphireSoc/include/soc.h"
	echo
	echo "Demo Ti60F225 with ethernet example design"
	echo "$ source init.sh ti60f225 $(pwd)/boards/efinix/ti60f225/hardware/ethernet/soc.h -e"
	echo
	echo "Demo Ti180J484 with singlecore example design"
	echo "$ source init.sh ti180j484 $(pwd)/boards/efinix/ti180j484/hardware/singlecore/soc.h"
	echo
	echo "Demo Ti180J484 with multicores example design"
	echo "$ source init.sh ti180j484 $(pwd)/boards/efinix/ti180j484/hardware/multicores/soc.h"
	echo
	echo "Demo Ti180J484 with unified hardware design"
	echo "$ source init.sh ti180j484 $(pwd)/boards/efinix/ti180j484/hardware/unified_hw/soc.h -u"
	echo
	echo "Demo Ti375C529 soc example design"
	echo "$ source init.sh ti375c529 $(pwd)/boards/efinix/ti375c529/hardware/soc/soc.h -e"
	echo
	echo "Demo Ti375C529 with unified hardware design"
	echo "$ source init.sh ti375c529 $(pwd)/boards/efinix/ti375c529/hardware/unified_hw/soc.h -u"
	echo
	echo "Demo Ti375c529 with unified hardware design + X11 graphics"
	echo "$ source init.sh ti375c529 $(pwd)/boards/efinix/ti375c529/hardware/unified_hw/soc.h -u -x"
	echo
	echo "Demo with eMMC u-boot configuration"
	echo "$ source init.sh ti375c529 $(pwd)/boards/efinix/ti375c529/hardware/soc/soc.h -w"
}

function sanity_check()
{
	title "Sanity Check"

	# check the compatible of $BOARD in $JSON_FILE
	local found=false
	local devkit_l
	BOARD=$(echo $BOARD | tr '[:upper:]' '[:lower:]')

	devkits=$(jq '.devkits.Titanium, .devkits.Trion | .[]' $JSON_FILE)

	for devkit in ${devkits[@]}; do
		devkit_l=$(echo $devkit | tr '[:upper:]' '[:lower:]'| sed 's/^"//;s/"$//')
		if [[ "$devkit_l" == "$BOARD" ]]; then
			BOARD=$(echo $devkit_l | tr -d \")
			BR2_DEFCONFIG="efinix_"$BOARD"_defconfig"
			found=true
			break
		fi
	done

	if [[ $found == false ]]; then
		pr_err "board $BOARD is not supported"
		return 1
	fi

	if [[ $UNIFIED_HW ]]; then
		if [[ $BOARD == "ti375c529" || $BOARD == "ti180j484" || $BOARD == "ti375n1156" ]]; then
			pr_info "board $BOARD support unified hardware design"
		else
			pr_err "board $BOARD does not support unified hardware design"
			return 1;
		fi
	fi

	if [ "$(grep SYSTEM_HARD_RISCV_QC32 $SOC_H | awk '{print $3}')" == "1" ]; then
		HARDEN_SOC=1
	fi
}

function check_soc_configuration()
{
	local smp_flag
	title "Checking Efinix RISC-V Sapphire SoC Configuration"

	get_cpu_count
        echo "INFO: Detecting $cpu_count RISC-V CPU cores"
        if [[ $cpu_count -gt 1 ]]; then
                # enable CONFIG_SMP=y in linux.config
                sed -i 's/^CONFIG_SMP=n/CONFIG_SMP=y/g' $BR2_EXTERNAL_DIR/boards/efinix/$BOARD/linux/linux.config
		smp_flag=1
        else
                sed -i 's/^CONFIG_SMP=y/CONFIG_SMP=n/g' $BR2_EXTERNAL_DIR/boards/efinix/$BOARD/linux/linux.config
		smp_flag=0
        fi
	self_check "Enable SMP support ..." "$smp_flag"

	# modify soc.h
	# remove last line '#endif'
	sed '/#endif/d' $SOC_H > ${SOC_H}.temp
	mv ${SOC_H}.temp $SOC_H

	# append SYSTEM_CORES_COUNT
	grep -q SYSTEM_CORES_COUNT $SOC_H || echo "#define SYSTEM_CORES_COUNT $cpu_count" >> $SOC_H

	# append addresses for AXI interconnect to soc.h
	if [ $UNIFIED_HW ] || [ $EXAMPLE_DESIGN ]; then

		sed -i '/SYSTEM_AXI_A_BMB/d' $SOC_H
		pr_info "Append addresses for AXI interconnect"
		if [[ $BOARD = "ti375c529" || $BOARD = "ti375n1156" ]]; then
			if [[ $MACHINE_ARCH = 64 ]]; then
				grep -q SYSTEM_AXI_SLAVE $SOC_H || \
				cat <<-EOF >> $SOC_H
				#define SYSTEM_AXI_SLAVE_0_IO_CTRL 0xe8000000
				#define SYSTEM_AXI_SLAVE_0_IO_CTRL_SIZE 0x800000
				#define SYSTEM_AXI_SLAVE_1_IO_CTRL 0xe8800000
				#define SYSTEM_AXI_SLAVE_1_IO_CTRL_SIZE 0x10000
				#define SYSTEM_AXI_SLAVE_2_IO_CTRL 0xe8810000
				#define SYSTEM_AXI_SLAVE_2_IO_CTRL_SIZE 0x10000
				#define SYSTEM_AXI_A_BMB 0xe8000000
				#define SYSTEM_AXI_A_BMB_SIZE 0x800000
				#define SYSTEM_AXI_B_BMB 0xe8800000
				#define SYSTEM_AXI_B_BMB_SIZE 0x10000
				#define SYSTEM_AXI_C_BMB 0xe8810000
				#define SYSTEM_AXI_C_BMB_SIZE 0x10000
				EOF
			else
				grep -q SYSTEM_AXI_SLAVE $SOC_H || \
				cat <<-EOF >> $SOC_H
				#define SYSTEM_AXI_SLAVE_0_IO_CTRL 0xe8000000
				#define SYSTEM_AXI_SLAVE_0_IO_CTRL_SIZE 0x1000000
				#define SYSTEM_AXI_SLAVE_1_IO_CTRL 0xe9000000
				#define SYSTEM_AXI_SLAVE_1_IO_CTRL_SIZE 0x10000
				#define SYSTEM_AXI_SLAVE_2_IO_CTRL 0xe9100000
				#define SYSTEM_AXI_SLAVE_2_IO_CTRL_SIZE 0x10000
				#define SYSTEM_AXI_SLAVE_3_IO_CTRL 0xe9200000
				#define SYSTEM_AXI_SLAVE_3_io_CTRL_SIZE 0x10000
				#define SYSTEM_AXI_SLAVE_4_IO_CTRL 0xe9300000
				#define SYSTEM_AXI_SLAVE_4_IO_CTRL_SIZE 0x10000
				#define SYSTEM_AXI_A_BMB 0xe8000000
				#define SYSTEM_AXI_A_BMB_SIZE 0x1000000
				#define SYSTEM_AXI_B_BMB 0xe9000000
				#define SYSTEM_AXI_B_BMB_SIZE 0x10000
				#define SYSTEM_AXI_C_BMB 0xe9100000
				#define SYSTEM_AXI_C_BMB_SIZE 0x10000
				#define SYSTEM_AXI_D_BMB 0xe9200000
				#define SYSTEM_AXI_D_BMB_SIZE 0x10000
				#define SYSTEM_AXI_E_BMB 0xe9300000
				#define SYSTEM_AXI_E_BMB_SIZE 0x10000
				EOF
			fi
		elif [ $BOARD = "ti180j484" ]; then
			grep -q SYSTEM_AXI_SLAVE $SOC_H || \
			cat <<-EOF >> $SOC_H
			#define SYSTEM_AXI_SLAVE_0_IO_CTRL 0xe1000000
			#define SYSTEM_AXI_SLAVE_0_IO_CTRL_SIZE 0x800000
			#define SYSTEM_AXI_SLAVE_1_IO_CTRL 0xe1800000
			#define SYSTEM_AXI_SLAVE_1_IO_CTRL_SIZE 0x10000
			#define SYSTEM_AXI_SLAVE_2_IO_CTRL 0xe1810000
			#define SYSTEM_AXI_SLAVE_2_IO_CTRL_SIZE 0x10000
			#define SYSTEM_AXI_A_BMB 0xe1000000
			#define SYSTEM_AXI_A_BMB_SIZE 0x800000
			#define SYSTEM_AXI_B_BMB 0xe1800000
			#define SYSTEM_AXI_B_BMB_SIZE 0x10000
			#define SYSTEM_AXI_C_BMB 0xe1810000
			#define SYSTEM_AXI_C_BMB_SIZE 0x10000
		EOF
		fi
	fi

	# append '#endif' to soc.h
	grep -q "#endif" $SOC_H || echo "#endif" >> $SOC_H

	# check for floating point from soc.h and modify buildroot defconfig
	fp=$(cat ${SOC_H} | grep FPU | awk  '{print $3}' | head -1)
	if [[ $fp == 0 ]]; then
		# disable the floating point
		pr_info "Disable floating point in $BR2_EXTERNAL_DIR/configs/$BR2_DEFCONFIG"
		sed -i 's/^BR2_RISCV_ISA_CUSTOM_RVF=y/BR2_RISCV_ISA_CUSTOM_RVF=n/g' $BR2_EXTERNAL_DIR/configs/$BR2_DEFCONFIG
		sed -i 's/^BR2_RISCV_ISA_CUSTOM_RVD=y/BR2_RISCV_ISA_CUSTOM_RVD=n/g' $BR2_EXTERNAL_DIR/configs/$BR2_DEFCONFIG
		sed -i 's/^BR2_RISCV_ABI_ILP32D=y/BR2_RISCV_ABI_ILP32=y/g' $BR2_EXTERNAL_DIR/configs/$BR2_DEFCONFIG

		pr_info "Disable Linux FPU support in $BR2_EXTERNAL_DIR/boards/efinix/$BOARD/linux/linux.config"
		sed -i 's/^CONFIG_FPU=y/CONFIG_FPU=n/g' $BR2_EXTERNAL_DIR/boards/efinix/$BOARD/linux/linux.config
	fi
	self_check "Floating point support ..." "$fp"

	# check for compressed extension from soc.h
	ext_c=$(cat ${SOC_H} | grep SYSTEM_RISCV_ISA_EXT_C | awk '{print $3}' | head -1)
	if [ $ext_c == 1 ]; then
		# enable compressed extension flag in buildroot defconfig
		pr_info "Enable compressed extension (RVC) in $BR2_EXTERNAL_DIR/configs/$BR2_DEFCONFIG"
		sed -i 's/BR2_RISCV_ISA_CUSTOM_RVC=n/BR2_RISCV_ISA_CUSTOM_RVC=y/g' $BR2_EXTERNAL_DIR/configs/$BR2_DEFCONFIG
		sed -i 's/CONFIG_RISCV_ISA_C=n/CONFIG_RISCV_ISA_C=y/g' $BR2_EXTERNAL_DIR/boards/efinix/$BOARD/linux/linux.config
	fi
	self_check "Compressed extension support ..." "$ext_c"

	# change the size of DDR to 1024MB due to limitation of physical DDR.
	ddr_size=$(grep SYSTEM_DDR_BMB_SIZE $SOC_H | awk '{print $3}')
	phy_ddr_size=0x40000000

	if (( $ddr_size > $phy_ddr_size )); then
		sed -i "s/$ddr_size/$phy_ddr_size/g" $SOC_H
	fi
}


function generate_device_tree() {
	local base_cmd="python3 $DT_DIR/device_tree_generator.py -m $MACHINE_ARCH"
	local end_cmd="$SOC_H $BOARD"

	local linux_dt_config="$DT_DIR/config/linux"
	local uboot_dt_config="$DT_DIR/config/uboot"

	local override_linux_dt=""
	local override_uboot_dt=""
	local hw_features=()

	# Set override config paths
	if [ "$UNIFIED_HW" ]; then
		override_linux_dt="$linux_dt_config/$MACHINE_ARCH/unified_hw"
		override_uboot_dt="$uboot_dt_config/$MACHINE_ARCH/unified_hw"
	elif [ "$EXAMPLE_DESIGN" ]; then
		override_linux_dt="$linux_dt_config/$MACHINE_ARCH/example_design"
		override_uboot_dt="$uboot_dt_config/$MACHINE_ARCH/example_design"
	fi

	# Parse extra hardware features
	[ -n "$HW_FEATURES" ] && IFS=',' read -ra hw_features <<< "$HW_FEATURES"

	# Helper to build device tree command
	build_dt_cmd() {
	local type=$1
	local config_dir=$2
	local generic_dir="$config_dir/generic"
	local override_dir=$3
	local cmd="$base_cmd -c $config_dir/drivers.json -c $config_dir/peripherals.json -c $DT_DIR/config/boards/$BOARD/memory.json"

	contains_feature() {
		local search=$1
		for f in "${hw_features[@]}"; do
			[ "$f" == "$search" ] && return 0
		done
		return 1
	}

	if [ "$type" == "linux" ]; then
		cmd+=" -c $generic_dir/reserved_memory.json"
	fi

	for feature in "${hw_features[@]}"; do
		case "$feature" in
		spi)
			if ! contains_feature "sdhc"; then
				cmd+=" -c $generic_dir/spi-mmc.json"
			fi
			[ "$type" == "linux" ] && cmd+=" -c $generic_dir/spi-nor.json"
			;;
		gpio) [ "$type" = "linux" ] && cmd+=" -c $generic_dir/gpio_led.json" ;;
		ethernet)
			[ "$type" = "linux" ] && {
				cmd+=" -c $generic_dir/ethernet.json"
				[ -n "$override_dir" ] && cmd+=" -c $override_dir/ethernet.json"
			}
			;;
		sdhc)
			cmd+=" -c $generic_dir/sdhc.json"
			[ -n "$override_dir" ] && cmd+=" -c $override_dir/sdhc.json"
			;;
		emmc)
			cmd+=" -c $generic_dir/emmc.json"
			[ -n "$override_dir" ] && cmd+=" -c $override_dir/emmc.json"
			;;
		fb)
			[ "$type" = "linux" ] && {
				cmd+=" -c $generic_dir/framebuffer.json"
				[ -n "$override_dir" ] && cmd+=" -c $override_dir/framebuffer.json"
			}
			;;
		usb)
			[ "$type" = "linux" ] && {
				cmd+=" -c $generic_dir/usb.json"
				[ -n "$override_dir" ] && cmd+=" -c $override_dir/usb.json"
			}
			;;
		evsoc)
			if [ "$type" = "linux" ]; then
				cmd+=" -c $generic_dir/evsoc.json"
				cp "$SOC_H" "$PROJ_DIR/kernel_modules/evsoc/src/soc.h"
			fi
			;;
		watchdog)
			if [ "$type" = "uboot" ]; then
				cmd+=" -c $generic_dir/watchdog.json"
			fi
			;;
	    esac
	done

	echo "$cmd $end_cmd $type"
	}

	# Generate U-Boot Device Tree
	title "Generate U-Boot Device Tree"
	local uboot_cmd
	uboot_cmd=$(build_dt_cmd "uboot" "$uboot_dt_config" "$override_uboot_dt")
	echo -e "DEBUG: Uboot device tree cmd:\n${uboot_cmd// -c /\\\n  -c }"
	eval "$uboot_cmd" || return 1
	pr_info "Copying $DT_DIR/output/uboot/$MACHINE_ARCH/uboot.dts to $EFINIX_DIR/$BOARD/u-boot/uboot.dts"
	cp $DT_DIR/output/uboot/$MACHINE_ARCH/uboot.dts $EFINIX_DIR/$BOARD/u-boot/uboot.dts

	# Generate Linux Device Tree
	title "Generate Linux Device Tree"
	local linux_cmd
	linux_cmd=$(build_dt_cmd "linux" "$linux_dt_config" "$override_linux_dt")
	echo -e "DEBUG: Linux device tree cmd:\n${linux_cmd// -c /\\\n  -c }"
	eval "$linux_cmd" || return 1

	pr_info "Copying $DT_DIR/output/linux/$MACHINE_ARCH/sapphire.dtsi to $COMMON_DIR/dts/sapphire.dtsi"
	cp $DT_DIR/output/linux/$MACHINE_ARCH/sapphire.dtsi $COMMON_DIR/dts/sapphire.dtsi
	pr_info "Copying $DT_DIR/output/linux/$MACHINE_ARCH/linux.dts to $EFINIX_DIR/$BOARD/linux/linux.dts"
	cp $DT_DIR/output/linux/$MACHINE_ARCH/linux.dts $EFINIX_DIR/$BOARD/linux/linux.dts
}

function prepare_buildroot_env()
{
	# prepare Buildroot build environment
	title "Prepare Buildroot Build Environment"

	# merge Buildroot defconfig
	local defconfig_fragments="$BR2_DEFCONFIG_DIR/riscv${MACHINE_ARCH}_fragment"

	list_fragments=(
		"base_defconfig"
		"efinix_${BOARD}_defconfig"
	)

	for fragment in ${list_fragments[@]};
	do
		defconfig_fragments+=" $BR2_DEFCONFIG_DIR/$fragment"
	done

	echo "DEBUG: defconfig_fragments = $defconfig_fragments"

	export CONFIG_="BR2"
	bash $BUILDROOT_DIR/support/kconfig/merge_config.sh -r -m ${defconfig_fragments}
	unset CONFIG_
	BR2_DEFCONFIG="efinix_${BOARD}_${MACHINE_ARCH}_defconfig"
	mv .config $BR2_DEFCONFIG_DIR/$BR2_DEFCONFIG

	mkdir $BUILD_DIR
	cd $BUILD_DIR && \
	make O=$PWD BR2_EXTERNAL=$BR2_EXTERNAL_DIR -C $BUILDROOT_DIR $BR2_DEFCONFIG

	# Add storage-specific u-boot fragment to the build configuration
	local storage_fragment=""
	if [ "$USE_EMMC_BOOT" = "1" ]; then
		pr_info "Adding eMMC u-boot configuration fragment to build config"
		storage_fragment="\$(BR2_EXTERNAL_EFINIX_PATH)/boards/efinix/common/u-boot/uboot_emmc_defconfig"
	else
		pr_info "Adding SD card u-boot configuration fragment to build config (default)"
		storage_fragment="\$(BR2_EXTERNAL_EFINIX_PATH)/boards/efinix/common/u-boot/uboot_sd_defconfig"
	fi

	# Append storage fragment to build configuration
	if grep -q "BR2_TARGET_UBOOT_CONFIG_FRAGMENT_FILES" .config; then
		# If fragment files line exists, append to it
		sed -i "/BR2_TARGET_UBOOT_CONFIG_FRAGMENT_FILES/ s|\"$| $storage_fragment\"|" .config
	else
		# If no fragment files line exists, add it with base and storage configs
		echo "BR2_TARGET_UBOOT_CONFIG_FRAGMENT_FILES=\"\$(BR2_EXTERNAL_EFINIX_PATH)/boards/efinix/common/u-boot/uboot_base_defconfig $storage_fragment\"" >> .config
	fi
}

function get_cpu_count()
{
	# count number of cpu core
	cpu_count=1
	for i in {4..1}; do
		num=$(( $i-1))
		substr="SYSTEM_PLIC_SYSTEM_CORES_${num}_EXTERNAL_INTERRUPT"
		if [[ $(cat $SOC_H | grep $substr) ]]; then
			cpu_count=$i
			break
		fi
	done
}

function get_column()
{
	# Read the VERSION file to get version
	local column="$1"
	local substr="$2"
	local arr=()
	while IFS= read -r line; do
		if [[ $line == *$substr* ]]; then
			IFS=' ' read -r -a arr <<< "$line"
		fi
	done < $INPUT_FILE
	echo ${arr[$column]}
}

function get_version()
{
	local substr="$1"
	version=$(get_column "1" "$substr")
	echo $version
}

function get_repo_url()
{
	local substr="$1"
	repo_url=$(get_column "0" "$substr")
	echo $repo_url
}

function set_kernel_config()
{
	local kernel_frag_dir="${COMMON_DIR}/kernel"
	local hw_features=""
	local br2_kernel_cfg_keyword="BR2_LINUX_KERNEL_CONFIG_FRAGMENT_FILES"
	local br2_linux_kernel_cfg="$kernel_frag_dir/linux.config $kernel_frag_dir/linux_rv${MACHINE_ARCH}.config"
	local feature

	# Read from SOC_H to get the hardware features
	local base_features=("spi" "i2c" "gpio" "watchdog")

	title "Set Kernel Configuration"

	for feature in "${base_features[@]}"; do
		if grep -i -q $feature $SOC_H; then
			pr_info "hardware feature: $feature"
			br2_linux_kernel_cfg+=" $kernel_frag_dir/$feature.config"
			HW_FEATURES+="$feature,"
		fi
	done

	HW_FEATURES+="${EXTRA_HW_FEATURES}"

	if [ -n "${EXTRA_HW_FEATURES}" ]; then
		IFS=',' read -ra hw_features <<< "${EXTRA_HW_FEATURES}"

		for feature in "${hw_features[@]}"; do
			pr_info "hardware feature: $feature"

			case "$feature" in
			spi)
				br2_linux_kernel_cfg+=" $kernel_frag_dir/spi.config"
				;;
			i2c)
				br2_linux_kernel_cfg+=" $kernel_frag_dir/i2c.config"
				;;
			gpio)
				br2_linux_kernel_cfg+=" $kernel_frag_dir/gpio.config"
				;;
			sdhc)
				br2_linux_kernel_cfg+=" $kernel_frag_dir/sdhc.config"
				;;
			ethernet)
				br2_linux_kernel_cfg+=" $kernel_frag_dir/ethernet.config"
				;;
			dma)
				br2_linux_kernel_cfg+=" $kernel_frag_dir/dma.config"
				;;
			fb)
				br2_linux_kernel_cfg+=" $kernel_frag_dir/framebuffer.config"
				;;
			usb)
				br2_linux_kernel_cfg+=" $kernel_frag_dir/usb.config"
				;;
			watchdog|wdt)
				br2_linux_kernel_cfg+=" $kernel_frag_dir/watchdog.config"
				;;
			emmc)
				br2_linux_kernel_cfg+=" $kernel_frag_dir/emmc.config"
				;;
			esac
		done
	fi

	pr_info "$br2_kernel_cfg_keyword=\"$br2_linux_kernel_cfg\""
	if grep -q ${br2_kernel_cfg_keyword} ${BR2_DEFCONFIG_DIR}/${BR2_DEFCONFIG}; then
		sed -i "/$br2_kernel_cfg_keyword/c\\$br2_kernel_cfg_keyword=\"$br2_linux_kernel_cfg\"" "${BR2_DEFCONFIG_DIR}/${BR2_DEFCONFIG}"
	else
		echo "$br2_kernel_cfg_keyword=\"$br2_linux_kernel_cfg\"" >> "${BR2_DEFCONFIG_DIR}/${BR2_DEFCONFIG}"
	fi
}

function add_packages()
{
        title "Add Packages"

	pr_info "Append ${BR2_DEFCONFIG_DIR}/extra_packages_fragment"
	cat ${BR2_DEFCONFIG_DIR}/extra_packages_fragment >> ${BR2_DEFCONFIG_DIR}/${BR2_DEFCONFIG}

        if [ $UNIFIED_HW ]; then
		pr_info "Append ${BR2_DEFCONFIG_DIR}/evsoc_fragment"
		grep -q EVSOC ${BR2_DEFCONFIG_DIR}/${BR2_DEFCONFIG} || \
		cat ${BR2_DEFCONFIG_DIR}/evsoc_fragment >> ${BR2_DEFCONFIG_DIR}/${BR2_DEFCONFIG}
	fi

        if [ $X11_GRAPHICS ]; then
		pr_info "Append ${BR2_DEFCONFIG_DIR}/x11_fragment"
                grep -q BR2_PACKAGE_DESKTOP_ENVIRONMENT ${BR2_DEFCONFIG_DIR}/${BR2_DEFCONFIG} || \
		cat ${BR2_DEFCONFIG_DIR}/x11_fragment >> ${BR2_DEFCONFIG_DIR}/${BR2_DEFCONFIG}

		# remove EVSOC packages
		grep -v "EVSOC" ${BR2_DEFCONFIG_DIR}/${BR2_DEFCONFIG} > ${BR2_DEFCONFIG_DIR}/${BR2_DEFCONFIG}.temp
		mv ${BR2_DEFCONFIG_DIR}/${BR2_DEFCONFIG}.temp ${BR2_DEFCONFIG_DIR}/${BR2_DEFCONFIG}
        fi
}

function parser()
{
	while [ $# -gt 0 ]
	do
		case $1 in
			-*) break
			;;
		esac
		shift
	done

	while getopts ":d:s:m:raehuxcw" o; do
		case "${o}" in
			:)
				pr_err "Option -$OPTARG requires an argument"
				return 1
				;;
			c)
				mod_files=$(git status | grep modified | awk '{print $2}')
				for f in ${mod_files[@]};
				do
					git checkout -- $f
				done
				pr_info "Repo reset to the original state"
				return
				;;
			d)
				OPT_DIR=${OPTARG}
				;;
			a)
				RECONFIGURE_ALL=1
				RECONFIGURE=1
				;;
			m)
				MACHINE_ARCH="${OPTARG}"
				if [ "$MACHINE_ARCH" != "64" ] && [ "$MACHINE_ARCH" != "32" ]; then
					pr_err "Unsupported machine architecture: ${MACHINE_ARCH}"
					return 1
				fi
				;;
			r)
				RECONFIGURE=1
				;;
			e)
				EXAMPLE_DESIGN=1
				;;
			u)
				UNIFIED_HW=1
				USE_EMMC_BOOT=1
				EXTRA_HW_FEATURES="sdhc,ethernet,evsoc,emmc,"
				;;
			s)
				EXTRA_HW_FEATURES+=${OPTARG}
				;;
			x)
				if [ "$UNIFIED_HW" = "1" ]; then
					X11_GRAPHICS=1
					EXTRA_HW_FEATURES="sdhc,ethernet,fb,dma,usb,emmc,"
				else
					pr_err "-x option requires -u to be set first."
					return 1
				fi
				;;
			w)
				USE_EMMC_BOOT=1
				;;
			h)
				usage
				return
				;;
			\?)
				pr_err "Invalid option -$OPTARG"
				return 1
				;;
			esac
	done
	shift $((OPTIND-1))
}

parser "$@"

if [[ -z $BOARD ]]; then
	pr_err "Board is not defined"
	return 1
fi

if [[ -z $SOC_H ]]; then
	pr_err "soc.h is not defined"
	return 1
elif [[ ! -f $SOC_H ]]; then
	pr_err "No such file for $SOC_H"
	return 1
fi

pr_info "Machine architecture: ${MACHINE_ARCH}-bit RISCV"

# clone sapphire-soc-dt-generator repository
if [ ! -d $DT_DIR ]; then
	dt_name="sapphire-soc-dt-generator"
	DT_REPO=$(get_repo_url "$dt_name")
	dt_version=$(get_version "$dt_name")
	title "Cloning $DT_REPO $dt_version"

        if [ ! -z $dt_version ]; then
                git clone $DT_REPO -b $dt_version $DT_DIR
        else
                git clone $DT_REPO $DT_DIR
        fi

	if [ ! $? -eq 0 ]; then
		pr_err "Failed to clone $DT_REPO"
		pr_err "Check you internet connection"
		return 1
	fi
fi

sanity_check || return 1

if [ $EXAMPLE_DESIGN ]; then
	if [ "$BOARD" = "ti60f225" ]; then
		EXTRA_HW_FEATURES+="ethernet,"
	fi

	if [[ "$BOARD" = "ti375n1156" ]]; then
		EXTRA_HW_FEATURES+="sdhc,ethernet,"
	fi

	if [[ "$BOARD" = "ti375c529" ]]; then
		EXTRA_HW_FEATURES+="sdhc,ethernet,emmc,"
	fi
fi

set_kernel_config
add_packages

WORKSPACE="build_$BOARD"

if [[ ! -z $OPT_DIR ]]; then
	WORKSPACE=$OPT_DIR
fi

WORKSPACE_DIR="$PROJ_DIR/../$WORKSPACE"
BUILDROOT_DIR="$PROJ_DIR/../$WORKSPACE/buildroot"
BUILD_DIR="$PROJ_DIR/../$WORKSPACE/build"
OPENSBI_DIR="$BR2_EXTERNAL_DIR/boards/efinix/$BOARD/opensbi"

if [[ $RECONFIGURE == 1 ]]; then
	title "Reconfigure Buildroot"
	# reconfigure the .config

	if [[ -d "$BUILD_DIR" ]]; then
		if [[ $RECONFIGURE_ALL == 1 ]]; then
			cp $SOC_H $OPENSBI_DIR/soc.h
			SOC_H=$OPENSBI_DIR/soc.h

			check_soc_configuration || return 1
			generate_device_tree || return 1
		fi
		prepare_buildroot_env || return 1
		return 0
	else
		pr_err "$BUILD_DIR not exist"
		return 1
	fi
fi

mkdir -p $WORKSPACE_DIR

if [ ! -d $BUILDROOT_DIR ]; then
	# clone buildroot repository
	BUILDROOT_REPO=$(get_repo_url "buildroot")
	buildroot_version=$(get_version "buildroot")
	title "Clone $BUILDROOT_REPO $buildroot_version"
	if [[ ! -z $buildroot_version ]]; then
		pr_info "Using Buildroot $buildroot_version"
		git clone $BUILDROOT_REPO -b $buildroot_version
	else
		echo Using Buildroot master/main branch
		git clone $BUILDROOT_REPO
	fi

	if [ ! $? -eq 0 ]; then
		pr_err "Failed to clone $BUILDROOT_REPO"
		pr_err "Check you internet connection"
		return 1
	fi

	mv buildroot $BUILDROOT_DIR

	title "Apply Patch"
	# apply out of tree patches for buildroot
	git -C $BUILDROOT_DIR reset --hard $buildroot_version
	if [ -d $BR2_EXTERNAL_DIR/patches/buildroot/$buildroot_version ]; then
		git -C $BUILDROOT_DIR am $BR2_EXTERNAL_DIR/patches/buildroot/$buildroot_version/*patch
	else
		pr_info "$BR2_EXTERNAL_DIR/patches/buildroot/$buildroot_version not exists. Skipping patch"
	fi
fi || return 1

title "Copy soc.h file to OpenSBI directory"
# copy soc.h to opensbi directory. Opensbi has dependency on soc.h.
pr_info "copy $SOC_H to $OPENSBI_DIR/soc.h"
cp $SOC_H $OPENSBI_DIR/soc.h
SOC_H=$OPENSBI_DIR/soc.h

check_soc_configuration || return 1
generate_device_tree || return 1
prepare_buildroot_env || return 1
