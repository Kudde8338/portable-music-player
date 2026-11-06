# [Name here]

An independent project originally forked from [@TotallyGarn47](https://github.com/TotallyGarn47).<br>
[The original project](https://github.com/TotallyGarn47/portable-music-player) and its contributors are credited for the work that served as the foundation for this project. See the original repository for its development history.


## About
A compact, self-contained MP3 player built around an **ESP32-D0WD-V3** and **Adafruit_PCM5122**. File decoding and digital audio playback based on pschatzmann's `arduino audio tools` and the likes. 

## Hardware

| Component | Notes |
|-----------|-------|
| ESP32-D0WD-V3 | Main computer |
| Adafruit MicroSD SPI or SDIO Card Breakout Board | SD-card module for file storage. Supports up to 32 GB. |
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
| CLK (MicroSD Breakout Board) | 18 |
| SO (MicroSD Breakout Board) | 19 |
| SI (MicroSD Breakout Board) | 23 |
| CS (MicroSD Breakout Board) | 05 |
| WSEL (PCM5122 I2S DAC) | 25 |
| DIN (PCM5122 I2S DAC) | 26 |
| BCK (PCM5122 I2S DAC) | 27 |

The DAC can also be pinned to the I²C SDA and SCL pins for filtering, de-emphasis, mute, as well as gain changes. To put into use, include the Adafruit_BusIO and Adafruit_PCM51xx libraries. See documentation [here](https://learn.adafruit.com/adafruit-pcm5122-i2s-dac/overview).

**NOTE:** Both the OLED and the MicroSD Breakout Board use 3.3V power input! This is especially important since both components **CAN** and **WILL** break if a higher voltage is used!
