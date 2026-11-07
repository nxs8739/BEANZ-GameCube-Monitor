# BEANZ GameCube System Monitor 1.0

A lightweight Nintendo GameCube homebrew diagnostic application written in C using devkitPro, devkitPPC, and libogc 3.2.1.

BEANZ GameCube System Monitor provides a simple on-console overview of system information and connected peripherals. It is designed to report information that can be collected through available hardware interfaces without presenting unsupported or misleading measurements.

## Features

### SYSTEM

- Configured MEM1 size
- Arena 1 span
- Allocator used chunks
- Allocator free chunks
- ARAM initialization status
- Video standard
- Video resolution
- Scan mode
- Nominal CPU clock
- Uptime
- Collection time
- VSync wait
- Frame interval

### DEVICES

- Controller status for ports 1–4
- Memory Card status for Slots A and B
- SP1 status
- SP2 status

SP1 and SP2 show `OCCUPIED` when a supported device responds to a detection
transaction and `NOT OCCUPIED` when no valid response is obtained. SP1 uses a
fresh EXI identification transaction. SP2 checks for a responding SD card
through SD2SP2, then attempts a standard EXI identification transaction.
These statuses do not identify attached device types. SP2 has no physical
presence-detect line, and passive adapters or unresponsive peripherals cannot
always be distinguished from empty ports. A temporary EXI transaction failure
retains the previous reading; the monitor does not claim universal physical
presence detection.

## Interface

- **L / R:** Switch between SYSTEM and DEVICES.
- **START:** Exit the monitor and return to Swiss.

The interface uses the GameCube console rendering facilities provided by libogc.

## Architecture

The project separates hardware access, snapshot collection, and rendering.

- **Hardware Abstraction Layers (HALs)** collect system and device information.
- **Snapshot structures** hold the collected information.
- **The renderer** displays the snapshot without independently querying hardware.

This keeps hardware access separate from presentation and provides a consistent view of collected system information.

## Building

### Requirements

- devkitPro
- devkitPPC
- libogc 3.2.1

### Build Instructions

Open a terminal in the project directory containing the `Makefile` and run:

```bash
make
```

The Makefile uses the devkitPro GameCube build rules to compile the project.

The build produces the GameCube DOL executable and ELF output in the project directory.

## Hardware Testing

Tested on a physical Nintendo GameCube DOL-001 running Swiss.

Hardware testing included:

- Successful application startup.
- SYSTEM and DEVICES page navigation.
- Controller connection and disconnection detection.
- Memory Card insertion and removal detection.
- Extended continuous runtime testing.
- Returning to Swiss using START.

An extended runtime test reached **11 hours and 20 minutes** without an observed crash or freeze.

Hardware results reflect the tested console and configuration. They are not a guarantee of identical behavior on every GameCube.

## Project Philosophy

The goal is straightforward: provide useful diagnostic information while keeping the application lightweight, readable, and honest about what it can actually detect.

## Credits

**Project direction, requirements, architecture, and hardware testing:** Beanz.

Development was AI-assisted, with technical research and review involving ChatGPT and Qwen.

Thanks to the GameCube homebrew community, devkitPro contributors, and the developers whose work makes GameCube homebrew possible.
