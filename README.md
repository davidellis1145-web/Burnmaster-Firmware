# Funnyplaying Burnmaster Firmware

Based on Sanni's [cartreader](https://github.com/sanni/cartreader) firmware.

Based off of v1.10 Burnmaster Firmware.

## Building the firmware
**Requires [SEGGER Embedded Studio (6.22a recommended)](https://www.segger.com/products/development-tools/embedded-studio/)** 

To build the firmware, open `GDCartReader.emProject` in Embedded Studio, then right click `Project 'GDCartReader'` and click `Build`. The compiled firmware can be found at `ProjectFolder/Debug/Exe/GDCartReader.bin`, you will need to rename this file to `update.bin` which can now be put at the root of your SD Card to update your BurnMaster

**Alternatively** you can simply clone this repository and make your changes, Github will automatically build your firmware (See Actions Tab)

### Repository Notes:

This repository is setup to auto-build the firmware when a commit is made, builds can be found under the [Actions tab](https://github.com/davidellis1145-web/Burnmaster-Firmware/actions).


This source code is heavily modified, I have nothing to do with funnyplaying or the development of the official firmware.

If you want to use my version, have fun, otherwise you can find the official source code over at [HDR's Repo](https://github.com/HDR/Burnmaster-Firmware)
