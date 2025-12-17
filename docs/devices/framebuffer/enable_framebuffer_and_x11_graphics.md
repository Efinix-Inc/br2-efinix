# How to enable framebuffer driver and X11 graphics

A framebuffer is a portion of RAM containing a bitmap that drives a video display. It represents the memory buffer that holds the pixel data for the screen, allowing the computer to render images, text, and other graphics.

We are using [unified hardware design](https://github.com/Efinix-Inc/EmbeddedSystem-Solution) for Ti375C529 devkit. Please note that [init.sh](../../../init.sh) script already handle all these steps for unified hardware design by passing arguments `-u -x` to the `init.sh` script. However, in this tutorial, we show on how to enable it from the scratch.

The display controller is connected to DMA1 channel 1 with `interrupt ID 16`. The framebuffer driver will start the transfer of bitmap from memory location `0x8000000` to the display controller using the DMA engine.

Here are the steps to enable framebuffer support.

1. Enable the framebuffer and DMA driver in th kernel configuration `br2-efinix/boards/efinix/ti375c529/linux/linux.config`.
   
   ```
   # Framebuffer
   CONFIG_FB=y
   CONFIG_FB_SIMPLE=y
   CONFIG_FRAMEBUFFER_CONSOLE=y
   CONFIG_FB_EFX=y
   
   # DMA
   CONFIG_DMADEVICES=y
   CONFIG_EFINIX_DMA=y
   ```

2. Edit the device tree to add DMA and framebuffer node.
   
   Edit `br2-efinix/board/efinix/common/dts/sapphire.dtsi` to add dma node.
   
   ```
   dma1: dma@100000 {
       reg = <0x100000 4096>;
       compatible = "efx,dma-controller";
       interrupt-parent = <&plic>;
       interrupts = <0 16>;
       clocks = <&apb_clock 0>;
       clock-frequency = <200000000>;
       status = "disabled";
   };
   ```
   
   Edit `br2-efinix/board/efinix/ti375c529/linux/linux.dts`.
   
   i) Append `,framebuffer` to `stdout-path` in `chosen` node.
   
   ii) Append `console=tty0` to `bootargs` in `chosen` node.
   
   This is the expected `chosen` node.
   
   ```
   chosen {
       stdout-path = "serial0:115200n8,framebuffer";
       bootargs = "rootwait console=ttySL0 earlycon root=/dev/mmcblk0p2 init=/sbin/init mmc_core.use_spi_crc=0 console=tty0";
   };
   ```
   
   iii) Add `framebuffer` node.
   
   ```
   framebuffer@8000000 {
       #address-cells = <1>;
       #size-cells = <0>;
       compatible = "efx,efx-fb";
       reg = <0x8000000 0x800000>;
       dmas = <&dma1 1>;
       dma-names = "display";
       width = <1920>;
       height = <1080>;
       stride = <7680>;
       format = "a8b8g8r8";
       clocks = <&apb_clock 0>;
       status = "okay";
   };
   ```
   
   iv) Add `&dma1` node.
   
   ```
   &dma1 {
       #address-cells = <1>;
       #size-cells = <0>;
       #dma-cells = <1>;
       dma-channels = <2>;
       status = "okay";
   
       dma-channel@0 {
           #address-cells = <1>;
           #size-cells = <0>;
           reg = <0>;
           dma-names = "anydevice";
           chan-priority = <0>;
       };
   
       dma-channel@1 {
           #address-cells = <1>;
           #size-cells = <0>;
           reg = <1>;
           dma-names = "display";
           chan-priority = <2>;
       };
   };
   ```

3. Add X11 packages. We are using `BR2_PACKAGE_DESKTOP_ENVIRONMENT` package as it already have some X11 configuration, packages and other configuration for desktop environment. Append the line into `br2-efinix/configs/efinix_ti375c529_defconfig`.
   
   ```
   BR2_PACKAGE_DESKTOP_ENVIRONMENT=y
   ```

4. Build the Linux image.

5. Start the Xserver manually after kernel booting up.
   
   ```
   startx&
   ```

6. Run any X11 application such as xterm terminal emulator.
   
   ```
   export DISPLAY=:0
   xterm
   ```
