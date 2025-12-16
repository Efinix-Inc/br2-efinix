# Accessing UART Terminal

Accessing UART terminal to interact with Linux.

## Prerequsite

### Software

As usual you need to install a software on the development machine to communicate with development board.

```bash
sudo apt-get install minicom
```

### Hardware

Please follow the [setup development board](setup_development_board.md) document for required hardware.

## Find the UART Port

### T120F324 Development board

In a terminal, type the command:

```bash
dmesg | grep ttyUSB
```

The terminal displays a series of messages about the attached devices.

```
usb <number>: <adapter> now attached to ttyUSB<number>
```

There are many USB-to-UART converter modules on the market. Some use an FTDI chip which displays a message similar to:

```
usb 3-3: FTDI USB Serial Device converter now attached to ttyUSB0
```

However, the T120F324 Development Board also has an FTDI chip and gives the same message. So if you have both the UART module and the board attached at the same time, you may receive three messages similar to:

```
usb 3-3: FTDI USB Serial Device converter now attached to ttyUSB0
usb 3-2: FTDI USB Serial Device converter now attached to ttyUSB1
usb 3-2: FTDI USB Serial Device converter now attached to ttyUSB2
```

In this case the second 2 lines `(marked by usb 3-2)` are the development board and the first line `(usb 3-3)` is the UART module.

Use the `minicom` to access the UART terminal.

```bash
sudo minicom -D /dev/ttyUSB0
```

### Ti60F225 Development board

In a terminal, type the command

```bash
ls /dev/ttyUSB*
```

The terminal displays a list of attached devices.

```bash
/dev/ttyUSB0 /dev/ttyUSB1 /dev/ttyUSB2 /dev/ttyUSB3
```

The UART is `/dev/ttyUSB2`. Use the `minicom` to access the UART terminal.

```bash
sudo minicom -D /dev/ttyUSB2
```

### Ti180J484 Development board

The UART is `/dev/ttyUSB0`. Use the `minicom` to access the UART terminal.

```bash
sudo minicom -D /dev/ttyUSB0
```

### Ti375C529 Development board

The UART is `/dev/ttyUSB2`. Use the `minicom` to access the UART terminal.

```bash
sudo minicom -D /dev/ttyUSB2
```
