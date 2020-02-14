# IOST app for Ledger Nano S
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

Follow this [link](https://ledger.readthedocs.io/en/latest/userspace/getting_started.html) to setup development SDK, toolchain, application loader.

## Get source
Get source code to your local workspace, by calling:
```bash
$ git clone https://git.sfxdx.ru/iost/ledger-app-iost.git
```

### Ubuntu Dependencies
Install the following packages :
```bash
$ sudo apt update && sudo apt install -y wget git cmake python3
```

### OSX Dependencies
It is recommended that you install brew and xcode. 

Additionally you will need to:
```bash
$ brew install libusb
```

## Build

To build the app, follow ALL of the instructions in the link below to get your BOLOS development environment set up.

https://ledger.readthedocs.io/en/latest/userspace/getting_started.html

Then, use the Makefile to build and load the app:
```bash
$ export BOLOS_SDK=sdk/nanos-secure-sdk
$ make
$ make load
```

Before compiling those applications, verify that the following environment variables are set

  - BOLOS_SDK : must point to [secure_sdk_dev](https://github.com/LedgerHQ/blue-secure-sdk/tree/master) that  has been cloned from this repository
  - BOLOS_ENV : must point to the location where the [toolchain](https://github.com/LedgerHQ/blue-devenv/tree/master) has been built

Latest vesrions of gcc and clang compilers can be found here:
* https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2019q4/gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2
* http://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz


# Use

## Run docker container
```bash
$ export APP_NAME=ledger-app-iost && \
  docker build -t $APP_NAME . && \
  docker run -it -p 4444:4444 -p 55555:55555 -v /dev:/dev --privileged --rm --name $APP_NAME $APP_NAME
```

If it doesn't meet any errors while building and running docker container then it makes binary as `bin/app.elf`.
And then you can get the binary from container by command:
```bash
$ docker cp ledger-app-iost:/home/bob/app/bin/app.elf ~/.
```

## Install Nano-S Application
To upload your application on the device, you should have virtual machine
or real machine with 64bit Linux. If you use virtual machine, then please
make sure that it can access your Nano-S device.

### 1. Change virtual machine or real machine udev rule.

Before you plug your device to the computer. You should add new rule
for the device.
Here is content of `/etc/udev/rules.d/80-nanos.rules` file.

    SUBSYSTEMS=="usb", ATTRS{idVendor}=="2c97", ATTRS{idProduct}=="0000", MODE="0666"
    SUBSYSTEMS=="usb", ATTRS{idVendor}=="2c97", ATTRS{idProduct}=="0001", MODE="0666"

To make the file, you should have administrator permission. Please use
`sudo` command for it.

### 2. Prepare device.

Prepare your Nano-S device, and connect it to the devie. And make it show
application selection menu, **DashBoard** application after some steps.
After successful detection you may confirm it with `lsusb` command.

    # lsusb
    Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub
    Bus 002 Device 021: ID 2c97:0001
    Bus 002 Device 002: ID 80ee:0021 VirtualBox USB Tablet
    Bus 002 Device 001: ID 1d6b:0001 Linux Foundation 1.1 root hub

You may see the device with the ID, **`2c97`**.

## Then upload app to the target device


### 3. Install application

Now you can install application with simple commands.
```bash
$ docker exec -it ledger-app-iost env BOLOS_SDK=sdk/nanos-secure-sdk make -C app load
```

If you want to remove application
```bash
$ docker exec -it ledger-app-iost env BOLOS_SDK=sdk/nanos-secure-sdk make -C app delete
```
