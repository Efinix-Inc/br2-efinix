# Configure the board

## Configure the ethernet

If the devkit connected to existing local network, DHCP server would assign the IP address for the devkit. Below are the steps to assign the static IP address.

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
   
   > If you are using Windows and ping is not responding, try to disable the firewall then retry.

## Enable SSH X11 forwarding and Root login

X11 forwarding, also known as ssh -X, is a feature of the SSH protocol that allows users to run graphical applications on a remote server and interact with them using their local display and I/O devices. By default, the ssh not allow login using root. It also disable the X11 forwarding due to security issue. However, user can override it using this configuration for development purpose. Please note that dropbear does not support X11 forwarding. Make sure the to use `BR2_PACKAGE_OPENSSH=y` in the `config/efinix_<devkit>_defconfig`.

1. Enable the configuration for X11 forwarding and Root login
   
   ```bash
   echo """\
   X11Forwarding yes
   PermitRootLogin yes""" >> /etc/ssh/sshd_config
   ```

2. Restart the sshd service
   
   ```bash
   /etc/init.d/S50sshd restart
   ```

3. Create an empty file for authentication
   
   ```bash
   touch /root/.Xauthority
   ```
