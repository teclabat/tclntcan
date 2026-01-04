# Tcl Interface to ESD NTCAN API

A Tcl interface to the NTCAN API for CAN (Controller Area Network) bus communication through ESD CAN hardware.

**Version:** 1.3.0 \
**Package:** `ntcan` \
**Namespace:** `ntcan::`

---

## Table of Contents

1. [Overview](#overview)
2. [Key Features](#key-features)
3. [Supported Hardware](#supported-hardware)
4. [Installation](#installation)
5. [Command Reference](#command-reference)
   - 5.1 [Network Management](#network-management)
   - 5.2 [Baudrate Configuration](#baudrate-configuration)
   - 5.3 [ID Filtering](#id-filtering)
   - 5.4 [Message Operations](#message-operations)
   - 5.5 [Status and Monitoring](#status-and-monitoring)
   - 5.6 [Queue Management](#queue-management)
   - 5.7 [Timeout Configuration](#timeout-configuration)
   - 5.8 [Abort Operations](#abort-operations)
6. [Usage Examples](#usage-examples)
7. [Error Handling](#error-handling)
8. [Best Practices](#best-practices)
9. [Troubleshooting](#troubleshooting)
10. [License](#license)

---

## Overview

The **ntcan** package provides Tcl bindings to the ESD NTCAN API, enabling Tcl applications to communicate with CAN (Controller Area Network) bus devices through ESD CAN hardware interfaces. The package wraps the native NTCAN C API and exposes it through a clean Tcl command interface in the `ntcan::` namespace.

CAN bus is a robust vehicle bus standard designed to allow microcontrollers and devices to communicate with each other without a host computer. It's widely used in automotive, industrial automation, medical equipment, and aerospace applications.

## Key Features

- **Full protocol support:**
  - CAN 2.0A (11-bit identifiers)
  - CAN 2.0B (29-bit extended identifiers)
  - CAN FD (Flexible Data-Rate) with up to 64 bytes per message
- **Flexible ID filtering:**
  - Single ID filtering
  - Range-based filtering for efficient multi-ID reception
  - Support for standard and extended identifiers
- **Configurable baudrates:**
  - Standard CAN 2.0 baudrates (10 kbit/s to 1 Mbit/s)
  - CAN FD extended baudrates with separate nominal and data phase rates
- **Message transmission and reception:**
  - Synchronous and asynchronous operations
  - Configurable transmit/receive queues
  - Binary data handling via Tcl byte arrays
- **Monitoring and diagnostics:**
  - Bus statistics and error counters
  - Controller status reporting
  - Queue depth monitoring
- **Multi-network support:** Operate multiple CAN interfaces simultaneously
- **Thread-safe:** When Tcl is compiled with thread support
- **Cross-platform:** Windows, Linux, and various real-time operating systems

## Supported Hardware

### ESD CAN Interface Families

- **C402 Series** - Latest esdACC-based CAN boards with CAN FD support
- **USB3 Series** - USB 3.0 CAN interfaces with CAN FD
- **U400 Series** - High-end USB interfaces
- **C200/C331/C405** - PCI/PCIe board families (passive and active)

### Operating Systems

- **Windows:** 10, 11 (32-bit and 64-bit)
- **Linux:** 32-bit and 64-bit (SocketCAN support available)
- **Real-time OS:** QNX 6/7, VxWorks 5/6/7, RTX64, TenAsys INtime, RTOS-32

### Driver Information

Free drivers and documentation are available from ESD:
- [https://esd.eu/en/support/can-driver](https://esd.eu/en/support/can-driver)

---

## Installation

```tcl
package require ntcan
```

### Requirements

**Build-time:**
- Tcl 8.6 or later
- C++11 compatible compiler
- ESD NTCAN SDK (development headers)
- Autoconf and GNU Make

**Runtime:**
- Tcl 8.6+
- ESD NTCAN Driver installed
- ESD NTCAN library:
  - Windows: `ntcan.dll`
  - Linux: `libntcan.so`

### Building from Source

```bash
cd D:\CM.tcltk\tclntcan
autoconf
./configure --with-tcl=/path/to/tcl/lib
make
make test
make install
```

---

## Command Reference

### Network Management

#### `ntcan::Scan`

Scans for all available CAN networks and displays detailed information.

**Syntax:**
```tcl
ntcan::Scan
```

**Returns:**

- Multi-line string with information about each detected CAN network:
  - Board ID (hardware identifier)
  - DLL version
  - Driver version
  - Firmware version
  - Hardware version
  - Board status
  - Feature flags

**Example:**
```tcl
puts "Available CAN networks:"
puts [ntcan::Scan]
```

---

#### `ntcan::Open`

Opens a CAN network for communication and returns a handle.

**Syntax:**
```tcl
set handle [ntcan::Open net mode txqueuesize rxqueuesize txtimeout rxtimeout]
```

**Parameters:**

- `net` - Logical network number (0-based). Use `ntcan::Scan` to discover available networks.
- `mode` - Open mode flags:
  - `0` - Standard mode
  - `0x00000001` - NTCAN_MODE_OVERLAPPED (asynchronous I/O on Windows)
  - `0x00000002` - NTCAN_MODE_OBJECT (object mode)
  - Additional flags can be combined using bitwise OR
- `txqueuesize` - Transmit queue size (number of messages). Use 0 if not transmitting.
- `rxqueuesize` - Receive queue size (number of messages). Recommended: 100 or more.
- `txtimeout` - Transmit timeout in milliseconds (0 = no timeout)
- `rxtimeout` - Receive timeout in milliseconds (0 = no timeout)

**Returns:**

- Integer handle for subsequent operations

**Errors:**

- Throws error if network not found or cannot be opened

**Example:**
```tcl
# Open network 0 with TX queue of 10, RX queue of 100
# TX timeout: 0 (no timeout), RX timeout: 1000ms (1 second)
set handle [ntcan::Open 0 0 10 100 0 1000]
```

---

#### `ntcan::Close`

Closes an open CAN handle and releases resources.

**Syntax:**
```tcl
ntcan::Close handle
```

**Parameters:**

- `handle` - CAN handle from `ntcan::Open`

**Returns:**

- Empty string on success

**Notes:**

- Aborts all pending operations
- Clears all filter settings
- Always close handles when finished to avoid resource leaks

**Example:**
```tcl
ntcan::Close $handle
```

---

### Baudrate Configuration

#### `ntcan::SetBaudrate`

Sets the CAN baudrate using standard CAN 2.0 baudrate indices.

**Syntax:**
```tcl
ntcan::SetBaudrate handle baudrate
```

**Parameters:**

- `handle` - CAN handle
- `baudrate` - Baudrate index:
  - `0` - 1000 kbit/s (1 Mbit/s)
  - `1` - 800 kbit/s
  - `2` - 500 kbit/s
  - `3` - 250 kbit/s
  - `4` - 125 kbit/s
  - `5` - 100 kbit/s
  - `6` - 50 kbit/s
  - `7` - 20 kbit/s
  - `8` - 10 kbit/s

**Returns:**

- Empty string on success

**Notes:**

- Must be set before adding IDs to the filter
- Must be set before transmitting or receiving messages

**Errors:**

- Throws error on invalid baudrate or hardware error

**Example:**
```tcl
# Set to 500 kbit/s (common industrial baudrate)
ntcan::SetBaudrate $handle 2
```

---

#### `ntcan::GetBaudrate`

Returns the current CAN baudrate index.

**Syntax:**
```tcl
set baudrate [ntcan::GetBaudrate handle]
```

**Parameters:**

- `handle` - CAN handle

**Returns:**

- Integer baudrate index (0-8)

**Example:**
```tcl
set baud [ntcan::GetBaudrate $handle]
puts "Current baudrate index: $baud"
```

---

#### `ntcan::SetBaudrateX`

Sets extended baudrate parameters for CAN FD operation.

**Syntax:**
```tcl
ntcan::SetBaudrateX handle mode flags nominalBaudrate dataBaudrate
```

**Parameters:**

- `handle` - CAN handle
- `mode` - CAN FD mode flags (see NTCAN API documentation)
- `flags` - Configuration flags
- `nominalBaudrate` - Baudrate index for arbitration phase (0-8)
- `dataBaudrate` - Baudrate index for data phase (0-8, CAN FD only)

**Returns:**

- Empty string on success

**Notes:**

- Required for CAN FD communication
- Nominal baudrate is used during arbitration phase
- Data baudrate is used during data phase (faster)

**Errors:**

- Throws error if hardware doesn't support CAN FD

**Example:**
```tcl
# Configure CAN FD: 500 kbit/s nominal, 2 Mbit/s data
ntcan::SetBaudrateX $handle 0x01 0 2 0
```

---

#### `ntcan::GetBaudrateX`

Returns the current CAN FD baudrate configuration.

**Syntax:**
```tcl
set config [ntcan::GetBaudrateX handle]
```

**Parameters:**

- `handle` - CAN handle

**Returns:**

- List containing four elements: `{mode flags nominalBaudrate dataBaudrate}`

**Example:**
```tcl
set config [ntcan::GetBaudrateX $handle]
lassign $config mode flags nominal data
puts "Mode: $mode, Nominal: $nominal, Data: $data"
```

---

### ID Filtering

CAN message filtering determines which message IDs will be received. By default, no messages are received until IDs are added to the filter.

#### `ntcan::IdAdd`

Adds a single CAN identifier to the receive filter.

**Syntax:**
```tcl
ntcan::IdAdd handle id
```

**Parameters:**

- `handle` - CAN handle
- `id` - CAN identifier:
  - Standard (11-bit): 0 to 0x7FF
  - Extended (29-bit): 0 to 0x1FFFFFFF with bit 29 set (0x20000000)

**Returns:**

- Empty string on success

**Example:**
```tcl
# Add standard ID 0x100
ntcan::IdAdd $handle 0x100

# Add extended ID 0x12345678
ntcan::IdAdd $handle [expr {0x12345678 | 0x20000000}]
```

---

#### `ntcan::IdRegionAdd`

Adds a range of consecutive CAN identifiers to the receive filter.

**Syntax:**
```tcl
ntcan::IdRegionAdd handle idStart idEnd
```

**Parameters:**

- `handle` - CAN handle
- `idStart` - First CAN identifier in range (inclusive)
- `idEnd` - Last CAN identifier in range (inclusive)

**Returns:**

- Empty string on success

**Notes:**

- More efficient than adding individual IDs
- Useful for monitoring multiple consecutive IDs

**Example:**
```tcl
# Receive all IDs from 0x100 to 0x1FF
ntcan::IdRegionAdd $handle 0x100 0x1FF

# Receive all standard IDs
ntcan::IdRegionAdd $handle 0 0x7FF
```

---

#### `ntcan::IdDelete`

Removes a single CAN identifier from the receive filter.

**Syntax:**
```tcl
ntcan::IdDelete handle id
```

**Parameters:**

- `handle` - CAN handle
- `id` - CAN identifier to remove

**Returns:**

- Empty string on success

**Example:**
```tcl
ntcan::IdDelete $handle 0x100
```

---

#### `ntcan::IdRegionDelete`

Removes a range of CAN identifiers from the receive filter.

**Syntax:**
```tcl
ntcan::IdRegionDelete handle idStart idEnd
```

**Parameters:**

- `handle` - CAN handle
- `idStart` - First CAN identifier in range
- `idEnd` - Last CAN identifier in range

**Returns:**

- Empty string on success

**Example:**
```tcl
ntcan::IdRegionDelete $handle 0x100 0x1FF
```

---

### Message Operations

#### `ntcan::Write`

Transmits a CAN 2.0 message on the bus.

**Syntax:**
```tcl
ntcan::Write handle id data
```

**Parameters:**

- `handle` - CAN handle
- `id` - CAN identifier (11-bit or 29-bit)
- `data` - Binary data (0-8 bytes for CAN 2.0). Use `binary format` to create.

**Returns:**

- Empty string on success

**Errors:**

- Throws error on timeout or transmission failure

**Example:**
```tcl
# Send message with ID 0x100 containing 4 bytes
set data [binary format c* {0x11 0x22 0x33 0x44}]
ntcan::Write $handle 0x100 $data

# Send 32-bit integer
set value 12345678
set data [binary format i $value]
ntcan::Write $handle 0x200 $data

# Send float and two integers
set data [binary format fii 3.14159 100 200]
ntcan::Write $handle 0x300 $data
```

---

#### `ntcan::Read`

Reads CAN 2.0 messages from the receive queue.

**Syntax:**
```tcl
set messages [ntcan::Read handle maxMessages]
```

**Parameters:**

- `handle` - CAN handle
- `maxMessages` - Maximum number of messages to read (typically 10-100)

**Returns:**

- List of alternating ID and data pairs: `{id1 data1 id2 data2 ...}`
- Empty list if no messages available within timeout

**Example:**
```tcl
# Read up to 10 messages
set result [ntcan::Read $handle 10]

# Process received messages
foreach {id data} $result {
    puts [format "ID: 0x%03X" $id]

    # Display as hex
    binary scan $data H* hex
    puts "Data (hex): $hex"

    # Display length
    puts "Length: [string length $data] bytes"

    # Decode as 4 bytes
    if {[string length $data] >= 4} {
        binary scan $data c4 bytes
        puts "Bytes: $bytes"
    }
}
```

---

#### `ntcan::WriteX`

Transmits a CAN FD message with extended format.

**Syntax:**
```tcl
ntcan::WriteX handle id mode data
```

**Parameters:**

- `handle` - CAN handle
- `id` - CAN identifier
- `mode` - Message mode flags:
  - Bit rate switching (BRS)
  - Error state indicator (ESI)
  - CAN FD format flag
- `data` - Binary data (0-64 bytes for CAN FD)

**Returns:**

- Empty string on success

**Errors:**

- Throws error on timeout or transmission failure

**Example:**
```tcl
# Send CAN FD message with 32 bytes
set data [binary format c* [lrepeat 32 0xFF]]
ntcan::WriteX $handle 0x100 0x01 $data

# Send CAN FD message with mixed data
set data [binary format iii8c 0x12345678 0xABCDEF00 0x11111111 \
    1 2 3 4 5 6 7 8]
ntcan::WriteX $handle 0x200 0x01 $data
```

---

#### `ntcan::ReadX`

Reads CAN FD messages from the receive queue.

**Syntax:**
```tcl
set messages [ntcan::ReadX handle maxMessages]
```

**Parameters:**

- `handle` - CAN handle
- `maxMessages` - Maximum number of messages to read

**Returns:**

- List of ID, mode, and data triplets: `{id1 mode1 data1 id2 mode2 data2 ...}`
- Empty list if no messages available within timeout

**Example:**
```tcl
# Read CAN FD messages
set result [ntcan::ReadX $handle 10]

# Process received messages
foreach {id mode data} $result {
    puts [format "ID: 0x%03X, Mode: 0x%02X" $id $mode]
    binary scan $data H* hex
    puts "Data: $hex ([string length $data] bytes)"

    # Check if CAN FD frame
    if {$mode & 0x01} {
        puts "This is a CAN FD frame"
    }
}
```

---

### Status and Monitoring

#### `ntcan::Status`

Returns detailed status information about the CAN interface.

**Syntax:**
```tcl
set status [ntcan::Status handle]
```

**Parameters:**

- `handle` - CAN handle

**Returns:**

- Multi-line string containing:
  - Board ID (hardware identifier)
  - DLL version (major.minor.patch)
  - Driver version
  - Firmware version
  - Hardware version
  - Board status (hexadecimal flags)
  - Feature flags (capabilities)

**Example:**
```tcl
puts "Interface Status:"
puts [ntcan::Status $handle]
```

---

#### `ntcan::GetBusStatistic`

Returns bus statistics including error counters and message counts.

**Syntax:**
```tcl
set stats [ntcan::GetBusStatistic handle]
```

**Parameters:**

- `handle` - CAN handle

**Returns:**

- Statistics data structure with bus performance metrics

**Example:**
```tcl
set stats [ntcan::GetBusStatistic $handle]
puts "Bus Statistics: $stats"
```

---

#### `ntcan::GetCtrlStatus`

Returns the current controller status including error states.

**Syntax:**
```tcl
set status [ntcan::GetCtrlStatus handle]
```

**Parameters:**

- `handle` - CAN handle

**Returns:**

- Status flags indicating:
  - Bus-off state
  - Error warning level
  - Error passive state

**Example:**
```tcl
set status [ntcan::GetCtrlStatus $handle]
puts "Controller Status: [format 0x%X $status]"

# Check for bus-off condition
if {$status & 0x80} {
    puts "WARNING: Bus is in bus-off state!"
}
```

---

### Queue Management

#### `ntcan::FlushRxFifo`

Flushes (clears) all messages from the receive FIFO queue.

**Syntax:**
```tcl
ntcan::FlushRxFifo handle
```

**Parameters:**

- `handle` - CAN handle

**Returns:**

- Empty string on success

**Use Case:**

- Discard old messages when you only want to process new messages

**Example:**
```tcl
# Clear any old messages
ntcan::FlushRxFifo $handle

# Now read only new messages
set messages [ntcan::Read $handle 10]
```

---

#### `ntcan::GetRxMsgCount`

Returns the number of messages currently waiting in the receive queue.

**Syntax:**
```tcl
set count [ntcan::GetRxMsgCount handle]
```

**Parameters:**

- `handle` - CAN handle

**Returns:**

- Integer count of messages available for reading

**Example:**
```tcl
set count [ntcan::GetRxMsgCount $handle]
if {$count > 100} {
    puts "Warning: RX queue is filling up ($count messages)"
}
```

---

#### `ntcan::GetTxMsgCount`

Returns the number of messages currently waiting in the transmit queue.

**Syntax:**
```tcl
set count [ntcan::GetTxMsgCount handle]
```

**Parameters:**

- `handle` - CAN handle

**Returns:**

- Integer count of messages pending transmission

**Example:**
```tcl
set count [ntcan::GetTxMsgCount $handle]
puts "Messages pending transmission: $count"
```

---

### Timeout Configuration

#### `ntcan::SetRxTimeout`

Sets the timeout for receive operations.

**Syntax:**
```tcl
ntcan::SetRxTimeout handle timeout
```

**Parameters:**

- `handle` - CAN handle
- `timeout` - Timeout in milliseconds (0 = infinite/blocking)

**Returns:**

- Empty string on success

**Typical Values:**

- 100 ms - Quick polling
- 1000 ms - Standard applications
- 0 - Blocking (wait forever)

**Example:**
```tcl
# Set 1 second timeout
ntcan::SetRxTimeout $handle 1000
```

---

#### `ntcan::GetRxTimeout`

Returns the current receive timeout.

**Syntax:**
```tcl
set timeout [ntcan::GetRxTimeout handle]
```

**Parameters:**

- `handle` - CAN handle

**Returns:**

- Timeout in milliseconds

**Example:**
```tcl
set timeout [ntcan::GetRxTimeout $handle]
puts "RX timeout: $timeout ms"
```

---

#### `ntcan::SetTxTimeout`

Sets the timeout for transmit operations.

**Syntax:**
```tcl
ntcan::SetTxTimeout handle timeout
```

**Parameters:**

- `handle` - CAN handle
- `timeout` - Timeout in milliseconds (0 = infinite)

**Returns:**

- Empty string on success

**Example:**
```tcl
# Set 500ms transmit timeout
ntcan::SetTxTimeout $handle 500
```

---

#### `ntcan::GetTxTimeout`

Returns the current transmit timeout.

**Syntax:**
```tcl
set timeout [ntcan::GetTxTimeout handle]
```

**Parameters:**

- `handle` - CAN handle

**Returns:**

- Timeout in milliseconds

**Example:**
```tcl
set timeout [ntcan::GetTxTimeout $handle]
puts "TX timeout: $timeout ms"
```

---

### Abort Operations

#### `ntcan::AbortRx`

Aborts any pending receive operations.

**Syntax:**
```tcl
ntcan::AbortRx handle
```

**Parameters:**

- `handle` - CAN handle

**Returns:**

- Empty string on success

**Use Case:**

- Cancel blocking receive operations
- Useful with overlapped (asynchronous) I/O

**Example:**
```tcl
# Abort any pending reads
ntcan::AbortRx $handle
```

---

#### `ntcan::AbortTx`

Aborts any pending transmit operations.

**Syntax:**
```tcl
ntcan::AbortTx handle
```

**Parameters:**

- `handle` - CAN handle

**Returns:**

- Empty string on success

**Use Case:**

- Cancel pending transmissions
- Clear transmit queue

**Example:**
```tcl
# Abort pending transmissions
ntcan::AbortTx $handle
```

---

## Complete Examples

### Example 1: Basic CAN Communication

```tcl
package require ntcan

# Scan for available networks
puts "Available CAN networks:"
puts [ntcan::Scan]

# Open CAN network 0
set handle [ntcan::Open 0 0 10 100 0 1000]
puts "Opened handle: $handle"

# Set baudrate to 500 kbit/s
ntcan::SetBaudrate $handle 2
puts "Baudrate set to 500 kbit/s"

# Add receive filter for ID 0x100
ntcan::IdAdd $handle 0x100
puts "Added ID 0x100 to filter"

# Send a message
set data [binary format c* {0x11 0x22 0x33 0x44 0x55 0x66 0x77 0x88}]
ntcan::Write $handle 0x200 $data
puts "Sent message with ID 0x200"

# Read messages (timeout: 1 second)
set messages [ntcan::Read $handle 10]
if {[llength $messages] > 0} {
    foreach {id data} $messages {
        binary scan $data H* hex
        puts [format "RX - ID: 0x%03X, Data: %s" $id $hex]
    }
} else {
    puts "No messages received"
}

# Cleanup
ntcan::Close $handle
puts "Closed handle"
```

---

### Example 2: Monitoring Multiple IDs

```tcl
package require ntcan

# Open interface
set handle [ntcan::Open 0 0 1 500 0 100]
ntcan::SetBaudrate $handle 2

# Monitor range of IDs
ntcan::IdRegionAdd $handle 0x100 0x1FF
puts "Monitoring IDs 0x100 to 0x1FF"

# Display status
puts "\nInterface Status:"
puts [ntcan::Status $handle]

# Monitor for 10 seconds
set endtime [expr {[clock seconds] + 10}]
set msgcount 0

puts "\nMonitoring CAN bus..."
while {[clock seconds] < $endtime} {
    set messages [ntcan::Read $handle 50]

    foreach {id data} $messages {
        incr msgcount
        binary scan $data H* hex
        set timestamp [clock format [clock seconds] -format %H:%M:%S]
        puts [format "%s - ID: 0x%03X, Len: %2d, Data: %s" \
            $timestamp $id [string length $data] $hex]
    }

    # Check queue depth
    set queuedepth [ntcan::GetRxMsgCount $handle]
    if {$queuedepth > 100} {
        puts "WARNING: RX queue depth high: $queuedepth"
    }
}

puts "\nReceived $msgcount messages in 10 seconds"
ntcan::Close $handle
```

---

### Example 3: CAN FD Communication

```tcl
package require ntcan

# Open interface
set handle [ntcan::Open 0 0 10 100 0 1000]

# Configure CAN FD: 500 kbit/s nominal, 2 Mbit/s data
ntcan::SetBaudrateX $handle 0x01 0 2 0
puts "CAN FD configured"

# Display configuration
set config [ntcan::GetBaudrateX $handle]
lassign $config mode flags nominal data
puts "Mode: $mode, Nominal: $nominal, Data: $data"

# Add receive filter
ntcan::IdAdd $handle 0x300

# Send CAN FD message with 32 bytes
set data [binary format c* [concat \
    {0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08} \
    {0x11 0x12 0x13 0x14 0x15 0x16 0x17 0x18} \
    {0x21 0x22 0x23 0x24 0x25 0x26 0x27 0x28} \
    {0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38}]]

set mode 0x01  ;# CAN FD frame
ntcan::WriteX $handle 0x300 $mode $data
puts "Sent CAN FD message with 32 bytes"

# Read CAN FD messages
set messages [ntcan::ReadX $handle 10]
foreach {id rxmode data} $messages {
    binary scan $data H* hex
    puts [format "RX - ID: 0x%03X, Mode: 0x%02X, %d bytes" \
        $id $rxmode [string length $data]]
    puts "Data: $hex"
}

ntcan::Close $handle
```

---

### Example 4: Error Handling and Diagnostics

```tcl
package require ntcan

proc open_can_safe {net} {
    if {[catch {
        set handle [ntcan::Open $net 0 1 100 0 1000]
        return $handle
    } err]} {
        puts stderr "Error opening CAN network $net: $err"
        return -1
    }
}

proc monitor_health {handle} {
    # Controller status
    if {[catch {
        set status [ntcan::GetCtrlStatus $handle]
        puts "Controller Status: [format 0x%X $status]"

        # Check error states
        if {$status & 0x80} {
            puts "  WARNING: Bus-off state!"
        }
        if {$status & 0x40} {
            puts "  WARNING: Error passive!"
        }
        if {$status & 0x20} {
            puts "  WARNING: Error warning!"
        }
    } err]} {
        puts stderr "Error getting status: $err"
    }

    # Queue depths
    set rxcount [ntcan::GetRxMsgCount $handle]
    set txcount [ntcan::GetTxMsgCount $handle]
    puts "Queue depths - RX: $rxcount, TX: $txcount"
}

# Main program
set handle [open_can_safe 0]
if {$handle == -1} {
    puts stderr "Failed to open CAN interface"
    exit 1
}

# Configure
if {[catch {
    ntcan::SetBaudrate $handle 2
    ntcan::IdRegionAdd $handle 0 0x7FF
    puts "Configuration successful"
} err]} {
    puts stderr "Configuration error: $err"
    ntcan::Close $handle
    exit 1
}

# Monitor health every 5 seconds
for {set i 0} {$i < 3} {incr i} {
    puts "\n=== Health Check [expr {$i + 1}] ==="
    monitor_health $handle
    after 5000
}

# Cleanup
ntcan::Close $handle
puts "\nCAN interface closed successfully"
```

---

### Example 5: High-Speed Data Logging

```tcl
package require ntcan

# Open log file
set logfile [open "can_log.txt" w]

# Open CAN interface
set handle [ntcan::Open 0 0 0 1000 0 10]
ntcan::SetBaudrate $handle 2
ntcan::IdRegionAdd $handle 0 0x7FF

puts "Logging CAN traffic to can_log.txt (press Ctrl+C to stop)"

# Log messages
set msgcount 0
while {1} {
    set messages [ntcan::Read $handle 100]

    foreach {id data} $messages {
        incr msgcount

        # Create timestamp
        set timestamp [clock format [clock milliseconds] \
            -format "%Y-%m-%d %H:%M:%S"]

        # Convert data to hex
        binary scan $data H* hex

        # Write to log
        puts $logfile [format "%s,0x%03X,%d,%s" \
            $timestamp $id [string length $data] $hex]

        # Console output every 100 messages
        if {$msgcount % 100 == 0} {
            puts "Logged $msgcount messages..."
        }
    }

    # Flush periodically
    if {$msgcount % 1000 == 0} {
        flush $logfile
    }
}

# Cleanup (on exit)
close $logfile
ntcan::Close $handle
```

---

### Example 6: Periodic Message Transmission

```tcl
package require ntcan

# Open interface
set handle [ntcan::Open 0 0 10 100 0 1000]
ntcan::SetBaudrate $handle 2

# Periodic transmission task
proc send_periodic {handle id period_ms} {
    global keep_sending

    if {!$keep_sending} return

    # Create message with incrementing counter
    global counter
    set data [binary format ii $counter [clock milliseconds]]

    if {[catch {
        ntcan::Write $handle $id $data
        incr counter
    } err]} {
        puts stderr "Transmit error: $err"
    }

    # Schedule next transmission
    after $period_ms [list send_periodic $handle $id $period_ms]
}

# Start periodic transmission
set counter 0
set keep_sending 1

puts "Sending periodic messages on ID 0x100 every 100ms"
puts "Press Enter to stop..."

send_periodic $handle 0x100 100

# Wait for user input
gets stdin

# Stop transmission
set keep_sending 0
puts "Stopped"

ntcan::Close $handle
```

---

## Error Handling

All commands that can fail will throw a Tcl error with a descriptive message including:
- The NTCAN function that failed
- The numeric error code
- A human-readable description

### Using catch for Error Handling

```tcl
if {[catch {
    set handle [ntcan::Open 0 0 1 100 0 1000]
} err]} {
    puts stderr "Error: $err"
    # Handle error appropriately
}
```

### Common Error Codes

- **NTCAN_HANDLE_INVALID** - Invalid handle parameter
- **NTCAN_NET_NOT_FOUND** - Specified network number not found
- **NTCAN_INSUFFICIENT_RESOURCES** - Out of memory or handles
- **NTCAN_CONTR_BUSY** - Controller is busy
- **NTCAN_RX_TIMEOUT** - No message received within timeout
- **NTCAN_TX_TIMEOUT** - Message could not be sent within timeout

### Error Handling Example

```tcl
proc safe_can_operation {handle id} {
    set retry_count 0
    set max_retries 3

    while {$retry_count < $max_retries} {
        if {[catch {
            set data [binary format i $retry_count]
            ntcan::Write $handle $id $data
            return 1  ;# Success
        } err]} {
            puts stderr "Attempt [expr {$retry_count + 1}] failed: $err"
            incr retry_count
            after 100  ;# Wait before retry
        }
    }

    return 0  ;# Failed after retries
}
```

---

## Best Practices

### 1. Always Close Handles

```tcl
# Good practice
set handle [ntcan::Open 0 0 1 100 0 1000]
# ... use handle ...
ntcan::Close $handle

# Better practice with error handling
if {[catch {
    set handle [ntcan::Open 0 0 1 100 0 1000]
    # ... use handle ...
} error]} {
    puts stderr "Error: $error"
}
# Cleanup always happens
catch {ntcan::Close $handle}
```

### 2. Use Appropriate Queue Sizes

```tcl
# For high-traffic applications
set handle [ntcan::Open 0 0 100 1000 0 10]

# For low-traffic applications
set handle [ntcan::Open 0 0 10 100 0 1000]

# Transmit-only application
set handle [ntcan::Open 0 0 10 0 0 0]
```

### 3. Set Baudrate Before Adding Filters

```tcl
# Correct order
set handle [ntcan::Open 0 0 1 100 0 1000]
ntcan::SetBaudrate $handle 2
ntcan::IdAdd $handle 0x100

# Incorrect - may fail
set handle [ntcan::Open 0 0 1 100 0 1000]
ntcan::IdAdd $handle 0x100  ;# ERROR: baudrate not set
ntcan::SetBaudrate $handle 2
```

### 4. Use Region Filters for Multiple IDs

```tcl
# Efficient - single operation
ntcan::IdRegionAdd $handle 0x100 0x1FF

# Less efficient - multiple operations
for {set id 0x100} {$id <= 0x1FF} {incr id} {
    ntcan::IdAdd $handle $id
}
```

### 5. Monitor Queue Depths

```tcl
# Periodically check queue status
proc check_queues {handle} {
    set rxcount [ntcan::GetRxMsgCount $handle]
    set txcount [ntcan::GetTxMsgCount $handle]

    if {$rxcount > 500} {
        puts "WARNING: RX queue filling up - $rxcount messages"
        # Consider increasing read frequency or queue size
    }

    if {$txcount > 50} {
        puts "WARNING: TX queue backing up - $txcount messages"
        # Bus may be congested
    }
}
```

### 6. Use Appropriate Timeouts

```tcl
# Quick polling - short timeout
ntcan::SetRxTimeout $handle 10

# Normal operation - medium timeout
ntcan::SetRxTimeout $handle 1000

# Blocking operation - no timeout
ntcan::SetRxTimeout $handle 0
```

### 7. Handle Binary Data Properly

```tcl
# Correct - use binary format
set data [binary format c* {1 2 3 4}]
ntcan::Write $handle 0x100 $data

# Correct - parse received data
set messages [ntcan::Read $handle 10]
foreach {id data} $messages {
    binary scan $data c* bytes
    # Process bytes...
}
```

---

## Troubleshooting

### Connection Failed

**Problem:** Error when opening CAN network

**Solutions:**
- Verify CAN hardware is connected
- Check that ESD NTCAN driver is installed
- Use `ntcan::Scan` to verify network number
- Ensure no other application has exclusive access
- Check Windows Device Manager or Linux dmesg for hardware issues

### Timeout Errors

**Problem:** NTCAN_RX_TIMEOUT or NTCAN_TX_TIMEOUT

**Solutions:**
- Verify bus is properly terminated (120 ohm resistors at both ends)
- Check baudrate matches other devices on bus
- Increase timeout values
- Verify another device is transmitting expected messages
- Check bus for electrical issues (cable integrity, grounding)

### No Messages Received

**Problem:** `ntcan::Read` returns empty list

**Solutions:**
- Verify IDs are added to filter: `ntcan::IdAdd` or `ntcan::IdRegionAdd`
- Check baudrate is set correctly
- Verify another device is transmitting on the bus
- Use oscilloscope or CAN analyzer to verify bus activity
- Check receive timeout is sufficient

### Baudrate Configuration Failed

**Problem:** Cannot set baudrate

**Solutions:**
- Ensure baudrate is set before adding IDs to filter
- Verify hardware supports requested baudrate
- For CAN FD, use `SetBaudrateX` instead of `SetBaudrate`
- Check that no messages are being transmitted during configuration

### Queue Overflow

**Problem:** Messages being lost

**Solutions:**
- Increase RX queue size in `ntcan::Open`
- Read messages more frequently
- Use region filters instead of many individual IDs
- Monitor queue depth with `ntcan::GetRxMsgCount`
- Consider using multiple handles for different ID ranges

### Permission Denied (Linux)

**Problem:** Cannot access CAN hardware

**Solutions:**
- Add user to appropriate group (typically `dialout`)
- Check device permissions: `ls -l /dev/can*`
- Run with sudo (not recommended for production)
- Configure udev rules for permanent access

---

## Technical Notes

### Binary Data Handling

All data is handled as Tcl byte arrays, preserving binary integrity. Use `binary format` and `binary scan` for creating and parsing CAN messages.

### Thread Safety

The package is thread-safe when Tcl is compiled with thread support. Each handle can be used from multiple threads, but synchronization is the application's responsibility.

### CAN 2.0 vs. CAN FD

- **CAN 2.0:** Maximum 8 bytes per message, use `Read`/`Write` commands
- **CAN FD:** Up to 64 bytes per message, use `ReadX`/`WriteX` commands
- CAN FD requires compatible hardware and proper baudrate configuration with `SetBaudrateX`

### Identifier Format

- **Standard (11-bit):** 0 to 0x7FF (2047)
- **Extended (29-bit):** Set bit 29 (0x20000000) to indicate extended ID
- Extended ID value: 0 to 0x1FFFFFFF (536,870,911)

### Performance Considerations

- Use region filters for multiple consecutive IDs
- Increase queue sizes for high-traffic applications
- Read multiple messages per call (e.g., `Read $handle 100`)
- Consider using separate handles for transmit and receive
- Monitor queue depths to prevent overflow

---

## See Also

- [ESD NTCAN API Manual](https://esd.eu) (CAN-API_Manual.pdf in SDK)
- [CAN Bus Wikipedia](https://en.wikipedia.org/wiki/CAN_bus)
- [CAN FD Overview](https://www.can-cia.org/can-knowledge/can/can-fd/)

---

## License

Copyright (c) 2014 Markus Halla \
Copyright (c) 2025 TERMA Technologies GmbH

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; If not, see [http://www.gnu.org/licenses/](http://www.gnu.org/licenses/)

---

## Version History

**1.3.0** (2025)
- C++11 support
- Full CAN FD support with ReadX/WriteX commands
- Extended baudrate configuration (SetBaudrateX/GetBaudrateX)
- Extended API coverage
- Windows static linking support for C++ runtime
- Comprehensive documentation and examples
