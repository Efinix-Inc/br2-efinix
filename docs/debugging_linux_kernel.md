How to Debug Linux Kernel on RISCV
========================

## Description

Debugging the Linux kernel using KGDB over serial UART. This guide for Efinix RISCV Sapphire SoC and Ti180J484.
This guide contain 2 parts which configuring the Linux and connect the Linux with debugger.

### Minimum Requirement

- Efinix RISCV Sapphire SoC must support atomic and compressed extension

### Part 1: Configuration Steps

In Part 1, we will enable KGDB and configuring kernel with debug symbol.

**Steps**

1. Clone br2-efinix repo
   
   ```bash
   git clone https://github.com/Efinix-Inc/br2-efinix.git
   ```

2. Run `init.sh` script
   
   ```bash
   cd br2-efinix
   source init.sh ti180j484 /path/to/soc.h
   ```

3. Add kernel config in `boards/efinix/ti180j484/linux/linux.config`.
   
   ```
   # Kernel Debugging
   CONFIG_DEBUG_KERNEL=y
   CONFIG_KGDB=y
   CONFIG_DEBUG_INFO=y
   CONFIG_FRAME_POINTER=y
   CONFIG_KGDB_SERIAL_CONSOLE=y
   CONFIG_KGDB_TEST=n
   CONFIG_KGDB_KDB=y
   CONFIG_RANDOMIZE_BASE=n
   CONFIG_WATCHDOG=n
   CONFIG_GDB_SCRIPTS=y
   CONFIG_DEBUG_MEMORY_INIT=y
   ```

4. Enable GDB in buildroot defconfig. For example for Ti180J484 add these line in `config/efinix_ti180j484_defconfig`
   
   ```
   # GDB
   BR2_PACKAGE_HOST_GDB=y
   BR2_PACKAGE_HOST_GDB_PYTHON3=y
   BR2_PACKAGE_HOST_GDB_TUI=y
   BR2_PACKAGE_HOST_GDB_SIM=y
   BR2_PACKAGE_GDB=y
   BR2_PACKAGE_GDB_SERVER=y
   BR2_GDB_VERSION_10=y
   BR2_PACKAGE_GDB_DEBUGGER=y
   ```

5. Rerun the `init.sh` script and build the linux image
   
   ```bash
   cd br2-efinix
   source init.sh ti180j484 /path/to/soc.h -r
   make -j$(nproc)
   ```

6. Flash the linux image to SD card and into the board. See doc [Flash Linux image into SD card](flash_linux.md).

### Part 2: Run KGDB on Linux

You are require to finish the part 1 before proceed with part 2. In part 2, we will open 3 terminals.
Terminal 1 - for running KDMX
Terminal 2 - for communication to the target board using minicom
Terminal 3 - for GDB

**Steps**

1. Open first terminal. Clone KDMX and build it
   
   ```bash
   git clone git://git.kernel.org/pub/scm/utils/kernel/kgdb/agent-proxy.git
   cd agent-proxy
   make
   ```
2. Start the KDMX.  The `/dev/ttyUSB0`is a UART port for the Ti180J484. See doc [Accessing UART Terminal](accessing_uart_terminal.md) for other board.
   
   ```bash
   ./kdmx -n -b115200 -p "/dev/ttyUSB0" -s  /tmp/kdmx_ports
   ```
   
   KDMX will output as follows
   
   ```bash
   /dev/pts/12 is slave pty for terminal emulator
   /dev/pts/13 is slave pty for gdb
   ```

>     Use <ctrl>C to terminate program

3. Open second terminal and connect to target board using serial terminal provided by KDMX
   
   ```bash
   minicom -D $(cat /tmp/kdmx_ports_trm) -w
   ```

4. Boot up the Linux on target board

5. Set tty console on target board
   
   ```bash
   echo ttySL0 > /sys/module/kgdboc/parameters/kgdboc
   ```

6. Crash the kernel to enter the debugger mode
   
   ```bash
   echo g > /proc/sysrq-trigger
   ```

7. It will appear as follow
   
   ```bash
   KDB >
   ```

8. Open third terminal on host to run GDB
   
   ```bash
   cd build/linux-v5.10
   ../../host/bin/riscv32-buildroot-linux-gnu-gdb \
   -ex 'set arch riscv:rv32' \
   -ex 'add-auto-load-safe-path .' \
   -ex 'file vmlinux' \
   -ex "target extended-remote $(cat /tmp/kdmx_port_gdb)"
   ```

9. After connect to the board, you can set the breakpoint and start to debug. Set the breakpoint at terminal 3.
   For example, set the breakpoint at UART driver.
   
   ```bash
   (gdb) break *spinal_lib_uart_tx_chars
   (gdb) continue
   ```

10. Try type anything on the terminal 2 and observed the GDB halt the Linux execution and break into the debugger. In the GDB terminal, issue command as follows
    
    ```bash
    bt - backtrace
    s - step into
    n - step over
    i b - show breakpoint
    c - continue
    ```
