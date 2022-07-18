# Flash Linux Image onto SD Card

1. Check the existing disk on the computer.
   
   ```bash
   sudo fdisk -l
   ```

2. Insert the SD card into the computer and run this command again to see the detected SD card. Note down the device name. For example `/dev/sdx`. If in doubt, remove the SD card and run the command again.
   
   ```bash
   sudo fdisk -l
   ```

3. Copy the `sdcard.img` to the SD card.
   
   > Warning: There is a significant risk of damage to your filesystem if you use the wrong /dev/sdx. Make sure you get it right!
   
   ```bash
   sudo dd if=sdcard.img of=/dev/sdx status=progress
   ```

4. Unmount the SD card
   
   ```bash
   sudo umount /dev/sdx
   ```
