# Setup Development Board

This guide show on how to setup the development board with UART and MicroSD card module. This setup only applicable for Trion T120F324 board.

## Prerequisite

- Development kit
  
  - Trion T120 BGA324 Development kit
  
  - Titanium Ti60F225 Development kit

- MicroSD card 1GB

- PMOD MicroSD card module*

- USB UART module*

- 3x male to female jumper cable*

- MIPI and LVDS expansion daughter card*

- 2x USB micro cable*

`Note: * only required for Trion T120F324.`

## Setup T120F324

Setup the UART and PMOD MicroSD card modules for T120F324

<img src="img/setup_t120f324.jpg" title="" alt="alt text" width="589">

Connect the MIPI and LVDS Expansion daughter card to the board at P2 header.

1. Connect male to female jumper wires to the daughter card and UART module:
   
   - Rx to pin 4
   
   - Tx to pin 2
   
   - GND to pin 6

2. Connect the UART module to your computer via USB micro cable.

3. Connect a USB micro cable to the board and to your computer.

4. Connect PMOD MicroSD card module to J12 PMOD header on the board.

## Setup Ti60F225

The Titanium Ti60 F225 Development Board has a USB-to-UART converter connected to the Ti60's GPIOL_01 and GPIOL_02 pins. To use the UART, simply connect a USB cable to the FTDI USB connector on the Titanium Ti60 F225 Development Board and to your computer.