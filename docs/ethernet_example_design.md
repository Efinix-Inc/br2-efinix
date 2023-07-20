# Ethernet Example Design

Ethernet example design for Ti60F225

- `boards/efinix/ti60f225/hardware/ethernet/ti60-tsemac-linux.zip`

### Board Setup

##### Prerequisite

- Development kits
  
  - Ti60F225

- [Ethernet daughter card](https://www.efinixinc.com/support/docsdl.php?s=ef&pn=ETHERNET-DC-UG)

- Ethernet cable CAT 5e

- MicroSD card

**Steps**

1. Conenct the Ethernet daughter card to P1 header of the Ti60 development board.
2. Set J8, J9, J10, J11 headers to 1.8V.
3. Set J5, J6 headers to 3.3V.
4. Set J7 header to 1.8V.
5. Connect the ethernet cable to ethernet daughter card and your PC.
6. Flash the Linux image into SD card.
7. Insert the SD card into a Ti60 development board and open the UART terminal.

### Build Linux with ethernet support

Br2-efinix provide different configurtion for the ethernet. User need to provide `-e` argument when run `init.sh` script.

```bash
source init.sh ti60 boards/efinix/ti60f225/hardware/ethernet/soc.h -e
make -j$(nproc)
```

### Configure the IP Address for the devkit

1. Boot up the devkit with Linux

2. Set the IP address
   
   ```bash
   ifconfig eth0 192.168.5.2 up
   ```

3. On host, set the IP address such as `192.168.5.3`

4. Run `ping` command to test the connection on devkit
   
   ```bash
   ping 192.168.5.3
   ```
   
   > If you are using Windows and ping is not responding, try to disable the firewall and ping again.


