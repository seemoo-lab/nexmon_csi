# Pi 3B+ and 4B &bull; Raspbian Buster &bull; Kernel v4.19.97

|||
|---|---|
|Device | Raspberry Pi 3B+ and 4B|
|Raspbian | [Raspbian Buster Lite 2020-02-13](https://downloads.raspberrypi.org/raspbian_lite/images/raspbian_lite-2020-02-14/)|
|Commit | [b52fca](https://github.com/seemoo-lab/nexmon_csi/commit/b52fca3abc18715d6d12692e531164b5d62a78fd)|
|Nexmon Commit | [f9db9a](https://github.com/seemoo-lab/nexmon/commit/f9db9abcac8f40a7f8a8408429e34e1c51f33c97)|
|Date | January 30, 2020|

 **Note**: [Release pi-buster-4.19.97-plus](https://github.com/zeroby0/nexmon_csi/tree/release-pi-buster-4.19.97-plus/releases/pi-buster-4.19.97-plus) has unmerged code to add stability and more features.


## Installation

* Burn [Raspbian Buster Lite 2020-02-13](https://downloads.raspberrypi.org/raspbian_lite/images/raspbian_lite-2020-02-14/) onto an empty SD card. You can use [Etcher](https://www.balena.io/etcher/).
* [Create an empty file called `ssh`](https://www.raspberrypi.org/documentation/remote-access/ssh/), without any extension, on the boot partition of the SD card.
* [SSH](https://www.raspberrypi.org/documentation/remote-access/ssh/) into the Pi.
* With `sudo raspi-config`, set WiFi Country, Time Zone, and then Expand File System.
* Reboot when asked to.

Install dependencies. Do **not** run `apt upgrade`.

* `sudo apt update`
* `sudo apt install git libgmp3-dev gawk qpdf bc bison flex libssl-dev make automake texinfo libtool-bin tcpdump tmux libncurses5-dev`
* `sudo reboot`

Get Kernel Headers

* `sudo wget https://raw.githubusercontent.com/RPi-Distro/rpi-source/master/rpi-source -O /usr/local/bin/rpi-source && sudo chmod +x /usr/local/bin/rpi-source && /usr/local/bin/rpi-source -q --tag-update`
* `rpi-source`
* `sudo reboot`

Install Nexmon_CSI
* `sudo su`
* `wget https://raw.githubusercontent.com/zeroby0/nexmon_csi/master/releases/pi3Bplus-buster-4.19.97/install.sh -O install.sh`
* `tmux new -c /home/pi -s nexmon 'bash install.sh | tee output.log'`

Your installation will happen in this tmux session. The right bottom corner will show the step running. Use `ctrl-b d` to detach, and `tmux attach-session -t nexmon` to re-attach.

## Usage

1. Use `makecsiparams` to generate a base64 encoded parameter string which is used to configure the extractor.
    ```
    mcp -c 157/80 -C 1 -N 1
    m+IBEQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==
    ```
    `makecsiparams` supports several other features like filtering data by Mac IDs or by first byte. Run `mcp -h` to see all available options.
2. `ifconfig wlan0 up`
3. `nexutil -Iwlan0 -s500 -b -l34 -vm+IBEQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==`
4. `iw dev wlan0 interface add mon0 type monitor`
5. `ip link set mon0 up`

Collect CSI by listening on UDP socket 5500, e.g. by using tcpdump: `tcpdump -i wlan0 dst port 5500`. There will be one UDP packet per configured core and spatial stream for each incoming frame matching the configured filter.

## Known issues
* CSI collection may stop after changing parameters several times.
* CSI collection may stop after collecting several samples.  If you have this problem, see [release pi-buster-4.19.97-plus](https://github.com/zeroby0/nexmon_csi/tree/release-pi-buster-4.19.97-plus/releases/pi-buster-4.19.97-plus)
