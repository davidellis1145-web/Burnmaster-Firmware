# Game Boy Flash Master Firmware.

Based on Sanni's [cartreader](https://github.com/sanni/cartreader) firmware.

Custom firmware for the Funnyplaying [BurnMaster](https://funnyplaying.com/products/portable-cart-flasher-burnmaster).

## Building the firmware
**Requires [SEGGER Embedded Studio (6.22a recommended)](https://www.segger.com/products/development-tools/embedded-studio/)** 

To build the firmware, open `GDCartReader.emProject` in Embedded Studio, then right click `Project 'GDCartReader'` and click `Build`. The compiled firmware can be found at `ProjectFolder/Debug/Exe/GDCartReader.bin`, you will need to rename this file to `update.bin` which can now be put at the root of your SD Card to update your BurnMaster

**Alternatively** you can simply clone this repository and make your changes, Github will automatically build your firmware (See Actions Tab)

### Repository Notes:

This repository is setup to auto-build the firmware when a commit is made, builds can be found under the [Actions tab](https://github.com/davidellis1145-web/Burnmaster-Firmware/actions).


This firmware is based on Funnyplaying's BurnMaster firmware v1.10 and is heavily modified. I have nothing to do with funnyplaying or the development of the official firmware (or hardware).

If you are looking for the official firmware, you can find it over at [HDR's Repo](https://github.com/HDR/Burnmaster-Firmware)
