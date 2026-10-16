# ESP32-C6 MP3 Player

A compact, self-contained MP3 player built around an **ESP32-C6**.

## Hardware

| Component | Notes |
|-----------|-------|
| ESP32-C6-DEV-KIT-NX | Pre-soldered headers included |
| Adafruit Micro SD SPI or SDIO Card Breakout Board - 3V ONLY! | SD-card module for file storage. Supports up to 32 gb. |
| 0.96" OLED Display | 128 × 64 px, I²C interface, white-on-black |

## Software Dependencies

| Library | Author | Purpose |
|---------|--------|---------|
| `lvgl` | kisvegabor | Full-featured gui library for the 0.96" OLED Display |

## Pin Map 

| Function | GPIO |
|----------|------|
| I²C SDA (OLED) | — |
| I²C SCL (OLED) | — |
| CLK (MicroSD Breakout Board) | GPIO19 |
| CMD (MicroSD Breakout Board) | GPIO18 |
| DATA0 (MicroSD Breakout Board) |
| DATA3 (MicroSD Breakout Board) |

## Getting Started

1. **Assemble** – connect the DFPlayer Mini to the ESP32-C6 UART pins and the OLED via I²C.
2. **Install libraries** – add `DFRobotDFPlayerMini` through the Arduino Library Manager.
3. **Flash** – open the project in the Arduino IDE (or PlatformIO), select the *ESP32C6 Dev Module* board, and upload.
4. **Play** – insert an SD card formatted as **FAT32** with MP3 files in the root folder.

## License

*(Add your chosen license, e.g. MIT, GPL-3.0)*