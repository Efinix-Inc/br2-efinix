# Ethernet Example Design

### Prerequsite

Efinity 2023.1 with patch 3 or later

### Supported Board

- Ti180J484

- Ti60F225

Ethernet example design for Ti180J484

- `boards/efinix/ti180j484/hardware/ethernet/ti180-tsemac-linux.zip`

Ethernet example design for Ti60F225

- `boards/efinix/ti60f225/hardware/ethernet/ti60-tsemac-linux.zip`

### Board Setup

##### Prerequisite

- Development kits
  
  - Ti180J484
  - Ti60F225

- [Ethernet daughter card](https://www.efinixinc.com/support/docsdl.php?s=ef&pn=ETHERNET-DC-UG)

- Ethernet cable CAT 5e

- MicroSD card

**Steps**

**Ti180J484**

1. Connect the FMC-to-QSE adapter card to Ti180J484 devkit

2. Connect the Ethernet daughter to J1 header of the FMC-to-QSE adapter card

**Ti60F225**

1. Conenct the Ethernet daughter card to P1 header of the Ti60 development board.
2. Set J8, J9, J10, J11 headers to 1.8V.
3. Set J5, J6 headers to 3.3V.
4. Set J7 header to 1.8V.
5. Connect the ethernet cable to ethernet daughter card and your PC.
6. Flash the Linux image into SD card.
7. Insert the SD card into a Ti60 development board and open the UART terminal.

### Build Linux with ethernet support

Br2-efinix provide different configurtion for the ethernet. User need to provide `-e` argument when run `init.sh` script.

For example to build Linux kernel with ethernet support for the following board

- **Ti60F225**
  
  ```bash
  source init.sh ti60 boards/efinix/ti60f225/hardware/ethernet/soc.h -e
  make -j$(nproc)
  ```

- **Ti180J484**
  
  ```bash
  source init.sh ti180 boards/efinix/ti180j484/hardware/ethernet/soc.h -e
  make -j$(nproc)
  ```

See the [document](configure_the_board.md) to configure the board.
