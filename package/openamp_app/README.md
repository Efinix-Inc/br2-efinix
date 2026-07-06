# OpenAMP_APP

This is a sample application to demonstrate inter-processor communication
between Linux and a bare-metal firmware using OpenAMP (remoteproc and RPMsg)
on the Efinix Sapphire RISC-V SoC. The Linux side runs on the master core
and sends messages to a remote core running the bare-metal echo firmware.
The remote core echoes every message back and the application verifies the
payload integrity.

## Prerequisite

The remote firmware is built from `software/standalone/openampEcho/` in the
Efinix embedded software repository. The bare-metal firmware is compile using
Efinity RISC-V Embedded Software IDE and the compiled binary must be available
on the target at `/lib/firmware/openampEcho.elf`.
The efinix_ti375_remoteproc.ko kernel module must be loaded before running the
application.

For example:
modprobe efinix_ti375_remoteproc.ko
echo openampEcho.elf > /sys/class/remoteproc/remoteproc0/firmware
echo start > /sys/class/remoteproc/remoteproc0/state


## Usage

```
First Arg:        rpmsg device to use. Default is /dev/rpmsg0
Second Arg:       number of messages to send. Default is 50

Example usage
         ./openamp_app
         ./openamp_app /dev/rpmsg0 100
```

## Example

Start the remote core and run the echo test. By default, it sends `50`
messages over `/dev/rpmsg0` and checks every reply.

```
modprobe efinix_ti375_remoteproc.ko
echo openampEcho.elf > /sys/class/remoteproc/remoteproc0/firmware
echo start > /sys/class/remoteproc/remoteproc0/state
./openamp_app
```

Output on the Linux terminal

```
root@buildroot:~# ./openamp_app
Packet #47 (Len: 257, Token: U) -> PASSED 
Packet #48 (Len: 496, Token: V) -> PASSED 
Packet #49 (Len: 4, Token: W) -> PASSED 
Packet #50 (Len: 15, Token: X) -> PASSED 
--------------------------------------------------------
Echo Test Complete. Total: 50, Success: 50, Failures: 0
```

Output on the remote core terminal

```
EEEEEEE  FFFFFFF  IIIII  NN   NN  IIIII  XX   XX
EE       FF         I    NNN  NN    I     XX XX
EEEEE    FFFFF      I    NN N NN    I      XXX
EE       FF         I    NN  NNN    I     XX XX
EEEEEEE  FF       IIIII  NN   NN  IIIII  XX   XX

FPGA Acceleration | RISC-V | OpenAMP
----------------------------------------

BareMetal: SHM_BASE        = 04200000
BareMetal: SHM_RSC_ADDR    = 04200000
BareMetal: SHM_VRING0_ADDR = 04202000
BareMetal: SHM_VRING1_ADDR = 04204000
BareMetal: resource table published
BareMetal: openamp_init OK -- creating demo endpoint
BareMetal: endpoint created at addr 30
BareMetal: NS announcement Completed, idling
```

Stop the remote core after the test

```
echo stop > /sys/class/remoteproc/remoteproc0/state
```
