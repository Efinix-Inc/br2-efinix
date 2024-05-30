# Demo Application

This document contain a few demo applications. Some of the demo might require the ethernet capabilities. See configure the board [document](configure_the_board.md) to do so.

1. **gpio_app**
   
   Demonstrate the LED blinking by configuring the gpio. This demo require the [gpio driver](../kernel_modules/gpio).
   
   ```bash
   modprobe gpio
   gpio_app
   ```

2. **cv2_detection**
   
   This is an example of OpenCV application that detect an object in the photo.
   It uses the caffee model for the object detection. See [cv2_detection](../package/cv2_detection) document for more details.
   
   ```bash
   cv2_detection
   ```

3. **tiny_encryption**
   
   Demonstrate the usage of custom instruction. The hardware design must support custom instruction of tiny_encryption to allow this program to run successfully. There are 2 functions to execute the encryption which using the software base and hardware base (custom instruction). The hardware function execute much faster as it leverage the hardware engine through custom instruction. The tiny_encryption only support for [Ti375C529 example design](../boards/efinix/ti375c529/hardware/soc). 
   
   ```bash
   tiny_encryption
   ```

4. **evsoc_camera**
   
   A simple http server which serve camera stream output over the web using mjpeg format. This application is dedicated for Edge Vision SoC (EVSoC). It require a custom driver support called evsoc. This demo only support for [Ti375C529 unified hardware design](../boards/efinix/ti375c529/hardware/unified_hw). See [evsoc_camera](../package/evsoc_camera) for more details.

5. **nmon**
   
   nmon is is a systems administrator, tuner, benchmark tool.  It can display the CPU, memory, network, disks (mini graphs or numbers), file systems, NFS, top processes, resources (Linux version & processors) and on Power micro-partition information.
   
   ```bash
   nmon
   ```

6. **ttyd**
   
   ttyd is a simple command-line tool for sharing terminal over the web.
   
   ```bash
   ttyd -p 8080 sh
   ```
   
   Then open http://localhost:8080 with a browser, you will get a sh shell. Replace the `localhost` with the board IP address.
