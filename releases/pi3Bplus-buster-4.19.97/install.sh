#!/bin/sh

function setStatus () {
  if { [ "$TERM" = "screen" ] && [ -n "$TMUX" ]; } then
    tmux set status-right "Status: $1"
  else
    echo "Status: $1"
  fi
}

setStatus "Removing wpasupplicant"
apt remove wpasupplicant -y
echo "denyinterfaces wlan0" >> /etc/dhcpcd.conf

setStatus "Downloading Nexmon"
git clone https://github.com/seemoo-lab/nexmon.git
cd nexmon
git checkout f9db9abcac8f40a7f8a8408429e34e1c51f33c97
NEXDIR=$(pwd)

setStatus "Building libISL"
cd $NEXDIR/buildtools/isl-0.10
autoreconf -f -i
./configure
make
make install
ln -s /usr/local/lib/libisl.so /usr/lib/arm-linux-gnueabihf/libisl.so.10

setStatus "Building libMPFR"
cd $NEXDIR/buildtools/mpfr-3.1.4
autoreconf -f -i
./configure
make
make install
ln -s /usr/local/lib/libmpfr.so /usr/lib/arm-linux-gnueabihf/libmpfr.so.4

setStatus "Setting up Build Environment"
cd $NEXDIR
source setup_env.sh
make

setStatus "Downloading Nexmon_CSI"
cd $NEXDIR/patches/bcm43455c0/7_45_189/
git clone https://github.com/seemoo-lab/nexmon_csi.git

setStatus "Building and installing Nexmon_CSI"
cd nexmon_csi
git checkout b52fca3abc18715d6d12692e531164b5d62a78fd
make install-firmware

setStatus "Installing makecsiparams"
cd utils/makecsiparams
make
ln -s $PWD/makecsiparams /usr/local/bin/mcp

setStatus "Installing nexutil"
cd $NEXDIR/utilities/nexutil
make
make install

setStatus "Setting up Persistance"
cd $NEXDIR/patches/bcm43455c0/7_45_189/nexmon_csi/
cd brcmfmac_4.19.y-nexmon
mv $(modinfo brcmfmac -n) ./brcmfmac.ko.orig
cp ./brcmfmac.ko $(modinfo brcmfmac -n)
depmod -a

touch /home/pi/COMPLETED
setStatus "Completed"