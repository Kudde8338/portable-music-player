# ESP32-C6 MP3 Player

A compact, self-contained MP3 player built around the **ESP32-C6** microcontroller. It reads audio tracks from an SD card, controls playback via a DFRobot DFPlayer Mini module, and displays track info on a small OLED screen.

## Hardware

| Component | Notes |
|-----------|-------|
| ESP32-C6-DEV-KIT-NX | Pre-soldered headers included |
| DFRobot DFPlayer Mini (MP3-TF-16P) | SD-card module for audio decoding |
| 0.96" OLED Display | 128 × 64 px, I²C interface, white-on-black |

## Software Dependencies

| Library | Author | Purpose |
|---------|--------|---------|
| `DFRobotDFPlayerMini` | DFRobot | Serial interface to the DFPlayer Mini board |

## Pin Map *(fill in after wiring)*

| Function | GPIO |
|----------|------|
| I²C SDA (OLED) | — |
| I²C SCL (OLED) | — |
| TX → DFPlayer | — |
| RX ← DFPlayer | — |
| SD-card detect | — |

## Getting Started

1. **Assemble** – connect the DFPlayer Mini to the ESP32-C6 UART pins and the OLED via I²C.
2. **Install libraries** – add `DFRobotDFPlayerMini` through the Arduino Library Manager.
3. **Flash** – open the project in the Arduino IDE (or PlatformIO), select the *ESP32C6 Dev Module* board, and upload.
4. **Play** – insert an SD card formatted as **FAT32** with MP3 files in the root folder.

## Controls

| Action | Trigger |
|--------|---------|
| Play / Pause | *(define button or serial command)* |
| Next / Prev track | *(define)* |
| Volume up / down | *(define)* |

## License

*(Add your chosen license, e.g. MIT, GPL-3.0)*