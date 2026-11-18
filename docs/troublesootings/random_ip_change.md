# Problem
### Device: RPI 4b 8 gb
### OS: Ubuntu 26.04 Server
### Conditions:
* possibly, router reloads as a result of power supply down (**not proven**)
* The RPI was shut down in the evening and turned back on the following morning 
* It's not possible to connect via SSH from an anotther computer using the IP address that worked yesterday
* after connecting a monitoe and a keyboard to the RPI and logging in, the `ip a` command showed that the device hab been assigned a new IP address on the internal ntwork.

# Solution

The plan is connect via <rpi_name>.local instead of IP address. On RPI:
```bash
systemctl is-active avahi-daemon
```
if service is no installed:
```bash
sudo apt update
sudo apt install avahi-daemon
sudo systemctl enable --now avahi-daemon
```
Check hostname:
```bash
hostnamectl --static
```
>>> your_rpi_hostname

now you can connect using:
```bash
ssh username@hostname.local
```

Also yo can simply connecting:
```bash
nano ~/.ssh/config
```

add new host:
<pre>
...
Host rpi
    HostName ypur_rpi_hostname.local
    User username
</pre>

Now you can connect using:
```bash
ssh rpi
```

