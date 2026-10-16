# Garn47s portable music player

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
| I²C SDA (OLED) | 06 |
| I²C SCL (OLED) | 07|
| CLK (MicroSD Breakout Board) | 19 |
| CMD (MicroSD Breakout Board) | 18 |
| DATA0 (MicroSD Breakout Board) | 20 |
| DATA3 (MicroSD Breakout Board) | 23 |

**NOTE:** Both the OLED and the MicroSD Breakout Board use 3.3V power input! Especially important since both components **CAN** and **WILL** break if a higher voltage is used!

## Getting Started

1. **Assemble** – connect the components as stated in the [Pin Map](#pin-map).
2. **Install and set up libraries** – add `lvgl` through the Arduino Library Manager and follow the proper steps to get it set up. Follow the guide [here](https://lvgl.io/docs/open/8.3/get-started/quick-overview.html#add-lvgl-into-your-project).
3. **Flash** – open the project in the Arduino IDE (or PlatformIO), select the *ESP32C6 Dev Module* board, and upload.
4. **Play** – insert an SD card formatted as **FAT32** with MP3 files in the root folder.