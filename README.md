# UART interface Configuration and Initialization

A simple C program for configuring and initializing a UART interface on Linux using the `termios` API, that transmit a test message, and listening for incoming data.

This program was developed as a coding challenge for [RISC-V ACT Framework Enablement and M-Mode Firmware Validation on Hardware Board (RISC-V Mentorship)](https://riscv.org/job/risc-v-act-framework-enablement-and-m-mode-firmware-validation-on-hardware-board-risc-v-mentorship/) program.

## Features

- Opens and configures a serial device (e.g., `/dev/ttyUSB0` or `/dev/ttyS0`).
- Sets I/O baud rate to **115200**, **8N1** data format
- Switches the terminal to **raw mode** (non‑canonical, no echo, no signals, no flow control).
- Sends a test message over the serial port.
- Waits up to **5 seconds** for incoming data using `poll()`.
- Prints received data to stdout.
- Graceful error handling for missing devices, permissions, and I/O failures.

## Requirements

- **Linux** with standard development tools (`gcc`, `make`).
- **socat** (only required for the automated test script).

Install `socat` if you want to run the test script:

```bash
sudo apt install socat
```

## Build

Build the executable using the provided `Makefile`:

```bash
make
```

clean with

```bash
make clean
```

## Usage

Run the program with the path to the serial device:

```bash
./uart_conf.o /path/to/serial/device
```

If no argument is provided, the program uses `/dev/ttyS0`.

## Testing with Virtual Terminals

An automated test script (`test_uart.sh`) is included to validate the program’s functionality without physical hardware. It uses `socat` to create a pair of virtual serial ports that communicate with each other.

```bash
make test
```

The script:

1. Creates a virtual bridge (`/tmp/ttyV0` ↔ `/tmp/ttyV1`).
2. Sends a message (`Acknowledge`) to one end.
3. Check that the message was read by the other end.
4. Reports **Validation Passed** on success, or **Validation Failed** otherwise.

Virtual ports are automatically closed and cleaned up after.

## Project Structure

```bash
.
├── Makefile
├── README.md
├── test_uart.sh
└── uart_conf.c
```
