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
$ make
$ make load
```

## Run Demo
A short python demo script is provided to test basic function.
Connect the Ledger device and open IOST app, and then run the script:
```
#python demo.py
```

