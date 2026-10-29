# Garn47's Portable Music Player 

A compact, self-contained MP3 player built around an **ESP32-C6**.

## Hardware

| Component | Notes |
|-----------|-------|
| ESP32-C6-DEV-KIT-NX | Main computer |
| Adafruit MicroSD SPI or SDIO Card Breakout Board | SD-card module for file storage. Supports up to 32 GB. |
| 0.96" OLED Display | 128 × 64 px, I²C interface, white-on-black. |
| Adafruit PCM5122 I2S DAC with Line Level Output - HW, I2C or SPI | Digital to audio conversion and output. |

## Software Dependencies

| Library | Author | Purpose |
|---------|--------|---------|
| `lvgl` | kisvegabor | Full-featured GUI library for the 0.96" OLED Display |
| `Adafruit_BusIO` | adafruit | This is a helper library to abstract away I2C, SPI, and 'generic transport' (e.g. UART) transactions and registers. |
| `Adafruit_PCM51xx`| adafruit | Arduino library for the Texas Instruments PCM51xx series of stereo audio DACs with I2C control interface. |
| `arduino-audio-tools`| pschatzmann | Header-only C++ library that lets microcontroller projects (ESP32, RP2040, etc.) read, process, and play audio using simple Arduino-style streams, with built-in codecs, effects, I2S/USB/network I/O, and sound generation. | 
| `unit_devlab_fork_arduino_libhelix` | UNIT-Electronics-MX | Arduino/CMake library wrapping lightweight Helix decoders. Includes formats such as MP3 and WAV. The library is also responsable for converting the files to raw PCM audio and stream it to I2S or a callback. |
| `libfoxenflac` | astoeckel | FLAC (Free Lossless Audio Codec) decoder written in C99. |

## Pin Map 

| Function | GPIO |
|----------|------|
| I²C SDA (OLED) | 06 |
| I²C SCL (OLED) | 07|
| CLK (MicroSD Breakout Board) | 00 |
| SO (MicroSD Breakout Board) | 01 |
| SI (MicroSD Breakout Board) | 10 |
| CS (MicroSD Breakout Board) | 11 |
| WSEL (PCM5122 I2S DAC) | 23 |
| DIN (PCM5122 I2S DAC) | 22 |
| BCK (PCM5122 I2S DAC) | 21 |
| MCK (PCM5122 I2S DAC) | 20 (technically left unused) |

The DAC can also be pinned to the I²C SDA and SCL pins for filtering, de-emphasis, mute, as well as gain changes. See documentation [here](https://learn.adafruit.com/adafruit-pcm5122-i2s-dac/overview).

**NOTE:** Both the OLED and the MicroSD Breakout Board use 3.3V power input! This is especially important since both components **CAN** and **WILL** break if a higher voltage is used!

## Getting Started

1. **Assemble** – connect the components as stated in the [Pin Map](#pin-map).
2. **Install and set up libraries** – add `lvgl` through the Arduino Library Manager and follow the proper steps to get it set up. Follow the guide [here](https://lvgl.io/docs/open/8.3/get-started/quick-overview.html#add-lvgl-into-your-project).
3. **Flash** – open the project in the Arduino IDE (or PlatformIO), select the *ESP32C6 Dev Module* board, and upload.
4. **Play** – insert an SD card formatted as **FAT32** with MP3 files in the root folder.