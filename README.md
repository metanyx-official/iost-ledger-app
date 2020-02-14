# IOST app for Ledger Nano S
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

Follow this [link](https://ledger.readthedocs.io/en/latest/userspace/getting_started.html) to setup development SDK, toolchain, application loader.

## Get source
Get source code to your local workspace, by calling:
```
git clone https://git.sfxdx.ru/iost/ledger-app-iost.git
```

### Ubuntu Dependencies
Install the following packages:
```
sudo apt update && apt-get -y install build-essential git sudo wget cmake libssl-dev libgmp-dev autoconf libtool
```

### OSX Dependencies
It is recommended that you install brew and xcode. 

Additionally you will need to:


```
brew install libusb
```

## Building

To build the app, follow ALL of the instructions in the link below to get your BOLOS development environment set up.

https://ledger.readthedocs.io/en/latest/userspace/getting_started.html

Then, use the Makefile to build and load the app:

```bash
$ export BOLOS_SDK=sdk/nanos-secure-sdk
$ make
$ make load
```

## Run Demo
A short python demo script is provided to test basic function.
Connect the Ledger device and open IOST app, and then run the script:
```
#python demo.py
```

Before compiling those applications, verify that the following environment variables are set

  - BOLOS_SDK : must point to [secure_sdk_dev](https://github.com/LedgerHQ/blue-secure-sdk/tree/master) that  has been cloned from this repository
  - BOLOS_ENV : must point to the location where the [toolchain](https://github.com/LedgerHQ/blue-devenv/tree/master) has been built

## Run docker container

```
#docker build -t ledger-app-iost . && \
 docker run -it -p 4444:4444 -p 55555:55555 -v /dev:/dev --privileged --rm --name ledger-app-iost ledger-app-iost
```
