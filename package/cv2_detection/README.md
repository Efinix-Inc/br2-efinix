# CV2 Detection

This is an example of OpenCV application that detect an object in the photo.
It uses the caffee model for the object detection.

## Build

1. Add the following Buildroot configuration in board defconfig file. For example, the target development board is `T120F324` uses `configs/efinix_t120f324_defconfig`.
   
   ```
   BR2_PACKAGE_OPENCV3_WITH_FFMPEG=y
   BR2_PACKAGE_OPENCV3_LIB_HIGHGUI=y
   BR2_PACKAGE_OPENCV3_DNN=y
   BR2_PACKAGE_CV2_DETECTION=y
   ```

2. Run the `init.sh` script. You will be changed the directory to `build_t120f324/build` after run the `init.sh` script.
   
   ```bash
   cd br2-efinix
   . init <board> </path/to/soc.h>
   ```

3. Apply the opencv patch file to the Buildroot repository.
   
   ```bash
   cd ../buildroot
   git am ../../br2-efinix/patches/buildroot/2021.05.3/0001-opencv3-enable-support-for-dnn-library.patch
   ```

4. Run the Buildroot command
   
   ```bash
   cd ../build
   make O=$PWD BR2_EXTERNAL=../../br2-efinix -C ../buildroot efinix_t120f324_defconfig
   ```

5. Build the package
   
   ```bash
   make -j$(nproc)
   ```
   
   

## Run CV2_Detection

Flash the Linux image `sdcard.img` to SD card and boot up.

Run this command to execute cv2_detection application.

```bash
cv2_detection
```
