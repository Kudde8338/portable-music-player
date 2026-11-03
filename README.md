# Portable Music Player 

A compact, self-contained MP3 player built around an **ESP32-C6** and Adafruit_PCM5122 DAC. File decoding and digital audio playback based on pschatzmann's `arduino audio tools` and the likes. 

## Hardware

| Component | Notes |
|-----------|-------|
| ESP32-C6-DEV-KIT-NX | Main computer |
| Adafruit MicroSD SPI or SDIO Card Breakout Board | SD-card module for file storage. Supports up to 32 GB. |
| 0.96" OLED Display | 128 × 64 px, I²C interface, white-on-black. |
| Adafruit PCM5122 I2S DAC with Line Level Output - HW, I2C or SPI | Digital to audio conversion and output. |

## Software Dependencies

| Library | Author | Purpose | GitHub | 
|---------|--------|---------|--------|
| u8g2 | olikraus | GUI library for the OLED Display | [github.com/olikraus/u8g2.git](https://github.com/olikraus/u8g2.git) |
| arduino audio tools| pschatzmann | Header-only C++ library that lets microcontroller projects (ESP32, RP2040, etc.) read, process, and play audio using simple Arduino-style streams, with built-in codecs, effects, I2S/USB/network I/O, and sound generation. | [github.com/pschatzmann/arduino-audio-tools.git](https://github.com/pschatzmann/arduino-audio-tools.git) |
| unit devlab fork arduino libhelix | UNIT-Electronics-MX | Arduino/CMake library wrapping lightweight Helix decoders. Includes formats such as MP3 and WAV. The library is also responsable for converting the files to raw PCM audio and stream it to I2S or a callback. | [github.com/UNIT-Electronics-MX/unit_devlab_fork_arduino_libhelix.git](https://github.com/UNIT-Electronics-MX/unit_devlab_fork_arduino_libhelix.git) |
| libfoxenflac | astoeckel | FLAC (Free Lossless Audio Codec) decoder written in C99. | [github.com/astoeckel/libfoxenflac.git](https://github.com/astoeckel/libfoxenflac.git) |

## Pin Map 

| Function | GPIO |
|----------|------|
| I²C SDA (OLED) | 06 |
| I²C SCL (OLED) | 07 |
| CLK (MicroSD Breakout Board) | 00 |
| SO (MicroSD Breakout Board) | 01 |
| SI (MicroSD Breakout Board) | 10 |
| CS (MicroSD Breakout Board) | 11 |
| WSEL (PCM5122 I2S DAC) | 23 |
| DIN (PCM5122 I2S DAC) | 22 |
| BCK (PCM5122 I2S DAC) | 21 |

The DAC can also be pinned to the I²C SDA and SCL pins for filtering, de-emphasis, mute, as well as gain changes. To put into use, include the Adafruit_BusIO and Adafruit_PCM51xx libraries. See documentation [here](https://learn.adafruit.com/adafruit-pcm5122-i2s-dac/overview).

**NOTE:** Both the OLED and the MicroSD Breakout Board use 3.3V power input! This is especially important since both components **CAN** and **WILL** break if a higher voltage is used!

## Getting Started

1. **Assemble** – connect the components as stated in the [Pin Map](#pin-map).
2. **Install and set up libraries** – add the pre-specified libraries. 
3. **Flash** – open the project in the Arduino IDE (or PlatformIO), select the *ESP32C6 Dev Module* board, and upload. If you're using a different microcontroller; go through the proper channels to adjust the source code, pinouts, and flash method. 
4. **Play** – insert an SD card formatted as **FAT32** with MP3 files in the root folder. You can create custom playlist folders as well if so please, just note the path in the code. 
