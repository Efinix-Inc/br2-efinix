## To enable SD host

Set the linux configuration in `linux.config`

```
 # MMC
CONFIG_MMC_SDHCI=y
CONFIG_MMC_SDHCI_PLTFM=y
CONFIG_MMC_SDHCI_EFX=y
CONFIG_MMC_SDHCI_IO_ACCESSORS=y
```

Set in the device tree `sapphire.dtsi`

```
mmc0: mmc@100000 {
 reg = <0x100000 0x10000>;
 compatible = "efx,sdhci";
 interrupt-parent = <&plic>;
 interrupts = <16>;
 clocks = <&apb_clock 0>;
 clock-frequency = <100000000>;
 status = "disabled";
};
```

Set in the device tree `linux.dts`

```
&mmc0 {
 #address-cells = <1>;
 #size-cells = <0>;
 status = "okay";
 bus-width = <4>;
 cap-sd-highspeed;
 no-sdio;
 no-mmc;
 max-frequency = <100000000>;
```
