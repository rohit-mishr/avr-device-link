# UART PC–MCU Communication Assignment

## Overview

This project implements reliable UART communication between a Windows PC application and an AVR microcontroller.

The PC sends a text message to the MCU. The MCU:

- Receives the message byte-by-byte over UART
- Immediately stores each byte in EEPROM
- Reads the stored data back
- Sends the data to the PC for verification

A device identification handshake ensures that the PC communicates with the correct MCU when multiple serial devices are connected.

---

# System Components

## PC Application

The PC application is written in standard C++ using Win32 serial APIs.

### Responsibilities

- Scan available COM ports
- Identify the correct MCU using a device ID handshake
- Send a message to the MCU
- Receive and verify the echoed response

### Key characteristics

- Uses Windows serial APIs (CreateFile, ReadFile, WriteFile)
- Automatically scans COM1 – COM20
- Performs end-to-end message verification

Implementation file:  
pcApp.cpp

---

## MCU Firmware

The firmware is written in bare-metal AVR C and compiled with the avr-gcc toolchain.

### Characteristics

- Direct UART register configuration
- Uses avr-libc EEPROM routines
- No Arduino framework or Arduino APIs
- Stores incoming UART data directly to EEPROM

Implementation file:  
firmware.c

---

# Communication Parameters

| Parameter | Value |
|----------|------|
| Interface | UART |
| Baud Rate | 2400 |
| Data Bits | 8 |
| Parity | None |
| Stop Bits | 1 |
| Message Terminator | `\n` (newline) |

---

# Communication Protocol

## 1. Device Identification

PC sends:

```
ID?\n
```

MCU responds:

```
ID:ALGO-UNO-V1\n
```

PC verifies that the received ID matches the expected device ID.

If the ID does not match, the port is ignored and scanning continues.

---

## 2. Data Transmission

After successful identification:

PC sends the message followed by newline:

```
<message>\n
```

MCU receives the message byte-by-byte via UART.

Each received byte is immediately written into EEPROM.

---

## 3. Data Echo and Verification

- MCU reads the stored data back from EEPROM.
- Data is transmitted back to the PC.
- PC compares the echoed message with the original message.

If both match → communication verified successfully.

---

# Firmware Design

## Key design decisions

- UART configured using AVR hardware registers
- Each byte is written directly to EEPROM
- No large RAM buffer required
- EEPROM boundary checks prevent overflow

Firmware supports two message types:

- Identification command
- Normal message storage

## Main features

- UART initialization
- UART transmit and receive routines
- EEPROM write-as-you-receive mechanism
- EEPROM readback for verification

---

# PC Application Design

## Automatic Port Detection

The application scans:

```
COM1 → COM20
```

It attempts communication with each port until the correct device is found.

## Device Verification

The PC confirms the device using the ID handshake protocol before sending data.

## Startup Delay

A 2-second delay is added after opening the COM port to allow the MCU to reset and initialize its UART.

## Data Integrity Verification

The PC compares the received echo with the original message to confirm that:

- No bytes were lost
- No corruption occurred

---

# Build and Flash Instructions

## Firmware Compilation

Target MCU:  
ATmega328P

Clock frequency:  
16 MHz

Compile using:

```bash
avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -Os firmware.c -o firmware.elf
```

Convert to HEX:

```bash
avr-objcopy -O ihex firmware.elf firmware.hex
```

---

## Flash Firmware

Flash using avrdude:

```bash
avrdude -c arduino -p m328p -P COMx -b 115200 -U flash:w:firmware.hex:i
```

Replace COMx with the correct serial port.

---

# PC Application Build

Compile with a Windows C++ compiler.

Example using g++:

```bash
g++ pcApp.cpp -o pcapp.exe
```

Run the application:

```bash
pcapp.exe ALGO-UNO-V1 "your message here"
```

Example:

```bash
pcapp.exe ALGO-UNO-V1 "Hello MCU"
```

---

# Project Workflow

```
PC Application
      │
      │  ID?\n
      ▼
MCU Firmware
      │
      │  ID:ALGO-UNO-V1\n
      ▼
PC verifies device
      │
      │  message\n
      ▼
MCU writes bytes to EEPROM
      │
      ▼
MCU reads EEPROM
      │
      │  echoed message
      ▼
PC verifies integrity
```
