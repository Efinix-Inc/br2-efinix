#!/usr/bin/env python3
"""
eMMC Programmer Tool
Partitions and programs images to eMMC device
"""

import sys
import os
import subprocess
import argparse
import time
import tempfile
import shutil
from pathlib import Path

class EMMCProgrammer:
    def __init__(self, device, server="192.168.31.222", use_scp=False, ssh_user="root", ssh_path="/", ssh_password=None):
        self.device = device
        self.server = server
        self.use_scp = use_scp
        self.ssh_user = ssh_user
        self.ssh_path = ssh_path
        self.ssh_password = ssh_password
        self.boot_mount = "/mnt/emmc_boot"
        self.root_mount = "/mnt/emmc_root"
        self.work_dir = "/root/emmc"

    def run_command(self, cmd, check=True, shell=True):
        """Execute a shell command and return the result"""
        print(f"Running: {cmd}")
        try:
            result = subprocess.run(cmd, shell=shell, check=check,
                                  capture_output=True, text=True)
            if result.stdout:
                print(result.stdout)
            return result
        except subprocess.CalledProcessError as e:
            print(f"Command failed: {e}")
            if e.stderr:
                print(f"Error: {e.stderr}")
            raise

    def verify_emmc_device(self):
        """Verify that the device is actually an eMMC and not SD card"""
        print(f"Verifying {self.device} is an eMMC device...")

        # Check if device exists
        if not os.path.exists(self.device):
            raise ValueError(f"Device {self.device} does not exist")

        # Get device info
        try:
            # Check if it's an eMMC by looking at the device path
            # eMMC devices typically appear as mmcblk1, while SD cards are mmcblk0
            device_name = os.path.basename(self.device)
            if not device_name.startswith('mmcblk'):
                raise ValueError(f"Device {self.device} is not an MMC device")

            # Additional verification: check if it's removable (SD cards are removable, eMMC is not)
            sys_path = f"/sys/block/{device_name}/removable"
            if os.path.exists(sys_path):
                with open(sys_path, 'r') as f:
                    removable = f.read().strip()
                if removable == '1':
                    print("WARNING: Device appears to be removable (likely SD card)")
                    response = input("Continue anyway? (y/N): ")
                    if response.lower() != 'y':
                        raise ValueError("Operation cancelled by user")

            # Get device size for verification
            result = self.run_command(f"fdisk -l {self.device}")
            print("Device verification passed")

        except Exception as e:
            raise ValueError(f"Failed to verify eMMC device: {e}")

    def create_partitions(self):
        """Create the partition table and partitions on eMMC"""
        print(f"Creating partitions on {self.device}...")

        # Unmount any existing partitions
        self.unmount_partitions()

        # Create partition table using fdisk
        fdisk_commands = """o
n
p
1
2048
133119
a
1
n
p
2
133120

w
"""

        # Write partition table
        process = subprocess.Popen(['fdisk', self.device],
                                 stdin=subprocess.PIPE,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE,
                                 text=True)
        stdout, stderr = process.communicate(fdisk_commands)

        if process.returncode != 0:
            print(f"fdisk output: {stdout}")
            print(f"fdisk error: {stderr}")
            raise RuntimeError("Failed to create partitions")

        print("Partitions created successfully")

        # Wait for kernel to recognize new partitions
        time.sleep(2)

        # Format partitions
        self.format_partitions()

    def format_partitions(self):
        """Format the partitions with appropriate filesystems"""
        print("Formatting partitions...")

        # Format boot partition (64MB) with ext2
        self.run_command(f"mkfs.ext2 -F {self.device}p1")

        # Format root partition (remaining space) with ext4
        self.run_command(f"mkfs.ext4 -F {self.device}p2")

        print("Partitions formatted successfully")

    def mount_partitions(self):
        """Mount the eMMC partitions"""
        print("Mounting partitions...")

        # Create mount points
        os.makedirs(self.boot_mount, exist_ok=True)
        os.makedirs(self.root_mount, exist_ok=True)

        # Mount partitions
        self.run_command(f"mount {self.device}p1 {self.boot_mount}")
        self.run_command(f"mount {self.device}p2 {self.root_mount}")

        print("Partitions mounted successfully")

    def unmount_partitions(self):
        """Unmount the eMMC partitions"""
        print("Unmounting partitions...")

        # Try to unmount (ignore errors if not mounted)
        for mount_point in [self.boot_mount, self.root_mount]:
            try:
                self.run_command(f"umount {mount_point}", check=False)
            except:
                pass

        # Also try to unmount device partitions directly
        try:
            self.run_command(f"umount {self.device}p1", check=False)
            self.run_command(f"umount {self.device}p2", check=False)
        except:
            pass

    def download_images(self):
        """Download images via SCP or TFTP"""
        if self.use_scp:
            print(f"Downloading images from SSH server {self.server}...")
        else:
            print(f"Downloading images from TFTP server {self.server}...")

        # Create work directory
        os.makedirs(self.work_dir, exist_ok=True)
        os.chdir(self.work_dir)

        # Download images
        images = ["uImage", "linux.dtb", "rootfs.tar"]

        for image in images:
            print(f"Downloading {image}...")

            if self.use_scp:
                self.download_with_scp(image)
            else:
                self.download_with_tftp(image)

            # Verify file was downloaded
            if not os.path.exists(image):
                raise RuntimeError(f"Failed to download {image}")

            # Show file size
            file_size = os.path.getsize(image)
            print(f"  Downloaded {image}: {file_size:,} bytes")

        print("All images downloaded successfully")

    def download_with_tftp(self, image):
        """Download using tftp with progress monitoring"""
        import threading
        import time

        # Remove existing file if present
        if os.path.exists(image):
            os.remove(image)

        # Start download in background
        download_thread = threading.Thread(
            target=self._run_tftp_download, args=(image,)
        )
        download_thread.start()

        # Monitor progress with size updates
        print("  Progress: ", end="", flush=True)
        last_size = 0
        last_update_time = time.time()
        start_time = time.time()
        update_interval = 2.0  # Update every 2 seconds

        while download_thread.is_alive():
            current_time = time.time()
            if os.path.exists(image):
                current_size = os.path.getsize(image)

                # Show progress update every 2 seconds or when size changes significantly
                if (current_time - last_update_time >= update_interval) or \
                   (current_size > last_size + 1024*1024):  # 1MB increments

                    if current_size > 0:
                        elapsed = current_time - start_time
                        speed = current_size / elapsed if elapsed > 0 else 0
                        size_mb = current_size / (1024*1024)

                        # Clear previous line and show current progress
                        print(f"\r  Progress: {size_mb:.1f} MB downloaded ({speed/1024:.1f} KB/s)...",
                              end="", flush=True)

                        last_update_time = current_time
                        last_size = current_size
                    else:
                        print(".", end="", flush=True)
            else:
                # File doesn't exist yet, show activity
                if current_time - last_update_time >= 1.0:
                    print(".", end="", flush=True)
                    last_update_time = current_time

            time.sleep(0.5)  # Check more frequently

        download_thread.join()

        # Show completion with timing
        elapsed_time = time.time() - start_time
        if os.path.exists(image):
            final_size = os.path.getsize(image)
            if final_size > 0:
                speed = final_size / elapsed_time if elapsed_time > 0 else 0
                size_mb = final_size / (1024*1024)
                print(f"\r  Progress: {size_mb:.1f} MB - Done! ({elapsed_time:.1f}s, {speed/1024:.1f} KB/s)")
            else:
                print("\r  Progress: Done!")
        else:
            print("\r  Progress: Failed!")

    def download_with_scp(self, image):
        """Download using SCP with progress monitoring"""
        import threading
        import time

        # Remove existing file if present
        if os.path.exists(image):
            os.remove(image)

        # Construct SCP command (using Dropbear's dbclient/scp)
        remote_path = f"{self.ssh_user}@{self.server}:{self.ssh_path}{image}"
        if self.ssh_password:
            # Use sshpass for password authentication with Dropbear
            scp_cmd = f"sshpass -p '{self.ssh_password}' scp -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null {remote_path} {image}"
        else:
            # Check if we have Dropbear or OpenSSH, try both approaches
            dropbear_key = os.path.expanduser("~/.ssh/id_dropbear_rsa")
            if os.path.exists(dropbear_key):
                # Use dbclient with key for Dropbear - use shell redirection properly
                scp_cmd = f"dbclient -i {dropbear_key} -y {self.ssh_user}@{self.server} 'cat {self.ssh_path}{image}' > {image}"
            else:
                # Fallback to standard scp
                scp_cmd = f"scp -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null {remote_path} {image}"

        # Start download in background
        download_thread = threading.Thread(
            target=lambda: self.run_command(scp_cmd)
        )
        download_thread.start()

        # Monitor progress with size updates
        print("  Progress: ", end="", flush=True)
        last_size = 0
        last_update_time = time.time()
        start_time = time.time()
        update_interval = 1.0  # Update every 1 second for SCP (faster than TFTP)

        while download_thread.is_alive():
            current_time = time.time()
            if os.path.exists(image):
                current_size = os.path.getsize(image)

                # Show progress update every 1 second or when size changes significantly
                if (current_time - last_update_time >= update_interval) or \
                   (current_size > last_size + 1024*1024):  # 1MB increments

                    if current_size > 0:
                        elapsed = current_time - start_time
                        speed = current_size / elapsed if elapsed > 0 else 0
                        size_mb = current_size / (1024*1024)

                        # Clear previous line and show current progress
                        print(f"\r  Progress: {size_mb:.1f} MB downloaded ({speed/1024:.1f} KB/s)...",
                              end="", flush=True)

                        last_update_time = current_time
                        last_size = current_size
                    else:
                        print(".", end="", flush=True)
            else:
                # File doesn't exist yet, show activity
                if current_time - last_update_time >= 0.5:
                    print(".", end="", flush=True)
                    last_update_time = current_time

            time.sleep(0.3)  # Check more frequently for SCP

        download_thread.join()

        # Show completion with timing
        elapsed_time = time.time() - start_time
        if os.path.exists(image):
            final_size = os.path.getsize(image)
            if final_size > 0:
                speed = final_size / elapsed_time if elapsed_time > 0 else 0
                size_mb = final_size / (1024*1024)
                print(f"\r  Progress: {size_mb:.1f} MB - Done! ({elapsed_time:.1f}s, {speed/1024:.1f} KB/s)")
            else:
                print("\r  Progress: Done!")
        else:
            print("\r  Progress: Failed!")

    def _run_tftp_download(self, image):
        """Helper to run tftp download using BusyBox tftp"""
        # Use larger block size for better performance
        self.run_command(f"tftp -g -b 1468 -r {image} -l {image} {self.server}")

    def program_images(self):
        """Program the downloaded images to eMMC"""
        print("Programming images to eMMC...")

        # Change to work directory
        os.chdir(self.work_dir)

        # Copy boot images
        print("Copying boot images...")
        self.run_command(f"cp uImage linux.dtb {self.boot_mount}/")

        # Extract rootfs
        print("Extracting rootfs...")
        os.chdir(self.root_mount)
        self.run_command(f"tar -xf {self.work_dir}/rootfs.tar ./")

        # Sync to ensure all data is written
        print("Syncing data...")
        self.run_command("sync")

        print("Images programmed successfully")

    def cleanup(self):
        """Clean up temporary files and unmount"""
        print("Cleaning up...")

        # Unmount partitions
        self.unmount_partitions()

        # Remove work directory
        if os.path.exists(self.work_dir):
            shutil.rmtree(self.work_dir)

        print("Cleanup completed")

    def program_emmc(self, create_partitions=True, download_images=True):
        """Main function to program eMMC"""
        try:
            # Verify device
            self.verify_emmc_device()

            if create_partitions:
                # Create and format partitions
                self.create_partitions()

            # Mount partitions
            self.mount_partitions()

            if download_images:
                # Download images
                self.download_images()

            # Program images
            self.program_images()

            print("\neMMC programming completed successfully!")

        except Exception as e:
            print(f"\nError during eMMC programming: {e}")
            raise
        finally:
            # Always cleanup
            self.cleanup()

def main():
    parser = argparse.ArgumentParser(description="eMMC Programmer Tool")
    parser.add_argument("device", help="eMMC device path (e.g., /dev/mmcblk1)")
    parser.add_argument("-s", "--server", default="192.168.31.222",
                       help="Server IP address (default: 192.168.31.222)")
    parser.add_argument("--no-partition", action="store_true",
                       help="Skip partition creation (use existing partitions)")
    parser.add_argument("--no-download", action="store_true",
                       help="Skip image download (use existing images in /root/emmc)")
    parser.add_argument("--list-devices", action="store_true",
                       help="List available MMC devices")
    parser.add_argument("--scp", action="store_true",
                       help="Use SCP instead of TFTP (much faster)")
    parser.add_argument("-u", "--ssh-user",
                       help="SSH username for SCP")
    parser.add_argument("-p", "--ssh-path",
                       help="Remote path for SCP (Example: /home/user/Projects/emmc_br2/build_ti375c529/build/images/)")
    parser.add_argument("--ssh-password",
                       help="SSH password for SCP authentication")

    args = parser.parse_args()

    if args.list_devices:
        print("Available MMC devices:")
        try:
            result = subprocess.run("ls -l /dev/mmcblk*", shell=True,
                                  capture_output=True, text=True)
            if result.stdout:
                print(result.stdout)
            else:
                print("No MMC devices found")
        except:
            print("Error listing devices")
        return

    # Check if running as root
    if os.geteuid() != 0:
        print("Error: This script must be run as root")
        sys.exit(1)

    try:
        programmer = EMMCProgrammer(
            device=args.device,
            server=args.server,
            use_scp=args.scp,
            ssh_user=args.ssh_user,
            ssh_path=args.ssh_path,
            ssh_password=args.ssh_password
        )
        programmer.program_emmc(
            create_partitions=not args.no_partition,
            download_images=not args.no_download
        )
    except Exception as e:
        print(f"Failed to program eMMC: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
