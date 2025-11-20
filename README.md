# Tcl NTCAN Extension

A Tcl extension providing bindings to the ESD NTCAN API for CAN (Controller Area Network) bus communication.

## Overview

This package provides a Tcl interface to ESD's NTCAN API, enabling Tcl applications to communicate with CAN bus devices through ESD CAN hardware interfaces. The extension wraps the native NTCAN C API and exposes it through a clean Tcl command interface.

## Features

- Full support for CAN 2.0 and CAN FD protocols
- Standard and extended CAN identifier filtering
- Configurable baudrates including CAN FD nominal and data rates
- Synchronous and asynchronous message transmission/reception
- Bus statistics and status monitoring
- Support for multiple CAN networks simultaneously
- Thread-safe operation (when Tcl is compiled with thread support)

## Requirements

### Software Requirements

- **Tcl 8.6 or later** - Required for Tcl stubs support
- **ESD NTCAN Driver** - Must be installed on the system
- **ESD NTCAN SDK** - Development headers and libraries
  - Headers: `ntcan/ntcan.h`
  - Library: `libntcan` (Windows: `ntcan.lib`/`ntcan.dll`)

### Hardware Requirements

- Compatible ESD CAN interface hardware (see ESD CAN Driver Support section below)

### Supported Platforms

- Windows 10/11 (32-bit and 64-bit)
- Linux (32-bit and 64-bit)
- Other platforms supported by ESD NTCAN drivers (QNX, VxWorks, RTX64, etc.)

## Installation

### Prerequisites

1. Install the ESD NTCAN driver for your platform from [https://esd.eu/en/support/can-driver](https://esd.eu/en/support/can-driver)

2. Install the ESD NTCAN SDK (typically located at `C:\Program Files\ESD\CAN\SDK` on Windows)

### Building from Source

This extension uses the Tcl Extension Architecture (TEA) build system:

```bash
# Generate the configure script (if needed)
autoconf

# Configure the build
./configure --with-tcl=/path/to/tcl/lib

# Build the extension
make

# Install (may require administrator/root privileges)
make install
```

#### Windows (MSYS2/MinGW) Specific

The extension is configured to link against the Tcl stubs library (`libtclstub86.a`) and uses static linking for C++ runtime libraries:

```bash
./configure --with-tcl=/path/to/tcl/lib
make
make install
```

## Usage

### Loading the Extension

```tcl
package require ntcan 1.3.0
```

All commands are available in the `ntcan::` namespace.

### Basic Example

```tcl
package require ntcan

# Scan for available CAN devices
ntcan::Scan

# Open CAN network 0
# Parameters: net mode txqueuesize rxqueuesize txtimeout rxtimeout
set handle [ntcan::Open 0 0 1 100 0 1000]

# Set baudrate to 500 kbit/s
ntcan::SetBaudrate $handle 2

# Add CAN ID 0x100 to receive filter
ntcan::IdAdd $handle 0x100

# Write a CAN message (ID: 0x100, data: 01 02 03 04)
ntcan::Write $handle 0x100 [binary format c* {1 2 3 4}]

# Read CAN messages
set result [ntcan::Read $handle 10]
foreach {id data} $result {
    puts "Received ID: [format 0x%X $id], Data: [binary scan $data H* hex; set hex]"
}

# Close the CAN handle
ntcan::Close $handle
```

## API Reference

### Network Management

#### `ntcan::Scan`
Scans for all available CAN networks and displays their information.

**Returns:** Text output with device information for each detected network

#### `ntcan::Open net mode txqueuesize rxqueuesize txtimeout rxtimeout`
Opens a CAN network for communication.

**Parameters:**
- `net` - Logical network number (0-based)
- `mode` - Open mode flags (see NTCAN API documentation)
- `txqueuesize` - Transmit queue size
- `rxqueuesize` - Receive queue size
- `txtimeout` - Transmit timeout in milliseconds (0 = no timeout)
- `rxtimeout` - Receive timeout in milliseconds (0 = no timeout)

**Returns:** CAN handle (integer)

#### `ntcan::Close handle`
Closes an open CAN handle.

**Parameters:**
- `handle` - CAN handle from `ntcan::Open`

**Returns:** None

### Baudrate Configuration

#### `ntcan::SetBaudrate handle baudrate`
Sets the CAN baudrate (CAN 2.0).

**Parameters:**
- `handle` - CAN handle
- `baudrate` - Baudrate index (see NTCAN API documentation for values)

#### `ntcan::GetBaudrate handle`
Gets the current CAN baudrate.

**Returns:** Baudrate index

#### `ntcan::SetBaudrateX handle mode flags nominalBaudrate dataBaudrate`
Sets extended baudrate parameters for CAN FD.

**Parameters:**
- `handle` - CAN handle
- `mode` - CAN FD mode
- `flags` - Configuration flags
- `nominalBaudrate` - Arbitration phase baudrate index
- `dataBaudrate` - Data phase baudrate index

#### `ntcan::GetBaudrateX handle`
Gets the current CAN FD baudrate configuration.

**Returns:** List of {mode flags nominalBaudrate dataBaudrate}

### ID Filtering

#### `ntcan::IdAdd handle id`
Adds a CAN identifier to the receive filter.

**Parameters:**
- `handle` - CAN handle
- `id` - CAN identifier (11-bit or 29-bit)

#### `ntcan::IdRegionAdd handle idStart idEnd`
Adds a range of CAN identifiers to the receive filter.

**Parameters:**
- `handle` - CAN handle
- `idStart` - First CAN identifier in range
- `idEnd` - Last CAN identifier in range

#### `ntcan::IdDelete handle id`
Removes a CAN identifier from the receive filter.

#### `ntcan::IdRegionDelete handle idStart idEnd`
Removes a range of CAN identifiers from the receive filter.

### Message Operations

#### `ntcan::Write handle id data`
Writes a CAN 2.0 message.

**Parameters:**
- `handle` - CAN handle
- `id` - CAN identifier
- `data` - Binary data (use `binary format` to create)

**Example:**
```tcl
ntcan::Write $handle 0x100 [binary format c* {0x12 0x34 0x56 0x78}]
```

#### `ntcan::Read handle maxMessages`
Reads CAN 2.0 messages from the receive queue.

**Parameters:**
- `handle` - CAN handle
- `maxMessages` - Maximum number of messages to read

**Returns:** List of {id1 data1 id2 data2 ...}

#### `ntcan::WriteX handle id mode data`
Writes a CAN FD message with extended format.

**Parameters:**
- `handle` - CAN handle
- `id` - CAN identifier
- `mode` - Message mode flags
- `data` - Binary data (up to 64 bytes for CAN FD)

#### `ntcan::ReadX handle maxMessages`
Reads CAN FD messages from the receive queue.

**Returns:** List of {id1 mode1 data1 id2 mode2 data2 ...}

### Status and Monitoring

#### `ntcan::Status handle`
Retrieves detailed status information about the CAN interface.

**Returns:** Multi-line text with:
- Board ID
- DLL version
- Driver version
- Firmware version
- Hardware version
- Board status
- Feature flags

#### `ntcan::GetBusStatistic handle`
Gets bus statistics.

**Returns:** Statistics data structure

#### `ntcan::GetCtrlStatus handle`
Gets controller status information.

**Returns:** Controller status flags

### Queue Management

#### `ntcan::FlushRxFifo handle`
Flushes the receive FIFO queue.

#### `ntcan::GetRxMsgCount handle`
Gets the number of messages in the receive queue.

**Returns:** Message count

#### `ntcan::GetTxMsgCount handle`
Gets the number of messages in the transmit queue.

**Returns:** Message count

### Timeout Configuration

#### `ntcan::SetRxTimeout handle timeout`
Sets the receive timeout.

**Parameters:**
- `timeout` - Timeout in milliseconds

#### `ntcan::GetRxTimeout handle`
Gets the current receive timeout.

**Returns:** Timeout in milliseconds

#### `ntcan::SetTxTimeout handle timeout`
Sets the transmit timeout.

#### `ntcan::GetTxTimeout handle`
Gets the current transmit timeout.

### Abort Operations

#### `ntcan::AbortRx handle`
Aborts pending receive operations.

#### `ntcan::AbortTx handle`
Aborts pending transmit operations.

## ESD CAN Driver Support

### Supported Hardware Families

ESD provides drivers for several CAN interface families:

- **C402** - Latest esdACC-based CAN boards (multiple versions)
- **USB3** - Latest USB CAN interfaces with CAN FD support
- **U400** - High-end USB interfaces
- **C200/C331/C405** - PCI/PCIe board families (passive and active)

### Supported Operating Systems

- **Windows:** 10, 11 (64-bit primary, 32-bit available)
- **Linux:** 32-bit and 64-bit (SocketCAN support for USB3)
- **Real-time OS:** QNX 6/7, VxWorks 5/6/7, RTX64, TenAsys INtime, RTOS-32

### Driver Downloads

Free drivers are available from ESD:
- [https://esd.eu/en/support/can-driver](https://esd.eu/en/support/can-driver)

The download site provides:
- Installation manuals
- NTCAN API documentation (C/C++ developers guide)
- Hardware ID reference tables
- Version-specific driver packages

### Documentation

Detailed API documentation is available in the ESD NTCAN SDK:
- **Location:** `C:\Program Files\ESD\CAN\SDK\doc\en` (Windows)
- **Manual:** `CAN-API_Manual.pdf` - Complete NTCAN API reference

## License

Copyright (c) 2014 Markus Halla
Copyright (c) 2025 TERMA Technologies GmbH

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; If not, see [http://www.gnu.org/licenses/](http://www.gnu.org/licenses/)

## Version History

- **1.3.0** - Current version
  - C++11 support
  - CAN FD support (ReadX/WriteX, SetBaudrateX/GetBaudrateX)
  - Extended API coverage
  - Windows static linking support for C++ runtime

## Support and Resources

- **ESD CAN Driver Support:** [https://esd.eu/en/support/can-driver](https://esd.eu/en/support/can-driver)
- **ESD Website:** [https://esd.eu](https://esd.eu)
- **NTCAN API Manual:** Available in ESD NTCAN SDK installation

## Troubleshooting

### Common Issues

1. **"Package ntcan not found"**
   - Ensure the extension is properly installed
   - Check that `pkgIndex.tcl` is in the Tcl library path
   - Verify installation with: `info loaded`

2. **"Cannot find ntcan library"**
   - Ensure ESD NTCAN driver is installed
   - On Windows, verify `ntcan.dll` is in system PATH
   - On Linux, verify `libntcan.so` is in library path

3. **"NTCAN canOpen() failed"**
   - Check that CAN hardware is connected
   - Verify driver installation with ESD diagnostic tools
   - Check that the specified network number is valid
   - Ensure no other application is using the CAN interface exclusively

4. **Build errors**
   - Verify NTCAN SDK is installed with development headers
   - Check that C++11 compiler is available
   - Ensure `ntcan/ntcan.h` is in include path

## Contributing

This is an open-source project released under GPL v2. Contributions, bug reports, and feature requests are welcome.
