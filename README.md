# Cross-Platform Terminal - SerIO 3.x
Software for Electronics Enthusiasts

This project was originally sourced from [SerIO](https://we.easyelectronics.ru/CADSoft/kross-platformennyy-terminal---serio-3x.html). I have only copied it to GitHub.

**SerIO 3.0** is a cross-platform terminal application designed to facilitate communication and data exchange between a PC and various electronic devices equipped with a UART interface. The application supports working with two physical serial port channels.

The transmitted data can be in various formats: character, decimal, hexadecimal, binary, and mixed formats, as well as patterns and arbitrary data files. The software also includes a function to transmit data to the port based on a timer with a specified source and time interval. Received data can be displayed in text format (character, decimal, hexadecimal, binary) and graphical format (oscillogram, diagram). Using profiles, you can quickly change the application settings for different tasks and equipment. There is no limit to the number of profiles that can be created. For analysis or data collection, various report files can be generated.

**Example of Data Display in Graphical Formats**

The SerIO application is free software distributed under the GNU GPLv3 or later, without any warranties. It is dynamically compiled but includes all necessary dependencies (libraries) for its operation.

Tested on the following operating systems:
- Linux(x64): Ubuntu 20, 22, Mint 20, 21, Fedora 20, Manjaro 32, ArchLinux 2021
- Windows(x32/x64): 7, 8, 10

## Installation Guide
- **Linux**: `SerIO-30.tar.xz (Current)`
  - Unpack the SerIO folder to your home directory `/home/user/` (replace "user" with your username).
  - To install, run in terminal: `~/SerIO/install.sh`
  - To uninstall, run in terminal: `~/SerIO/uninstall.sh`
  - Additional information can be found in the file: `/SerIO/READMY.txt`

- **Windows**: `SerIO-30.exe`
- **Mac OS**: In preparation… Assistance with building for MacOS is highly appreciated!

### Changes in SerIO 3.0:

- Implemented dual-channel port operation mode.
- Added single-frame data reception-display mode.
- Added channel markers and data exchange time output.
- Dynamic resizing of the application window implemented.
- Redesigned data transmission-reception-display algorithms.
- Improved alignment of data in text formats DEC, HEX, BIN.

