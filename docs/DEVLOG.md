### __Day 1__
__10/16/2026 and 10/17/2026__

Starting the log today! Switched up the plan/system for what i am building. The previous plan was to use a DFplayer mini, but upon discovering the horrid audio quality I've decided to eventually purchase a dedicated DAC and work off of reading and processing the files on the SD card through a dedicated reader. 

My current doubts regard:
* The model of esp I'm working with only supports low power bluetooth, so no audio capabilities. 
* The model of esp I'm working with doesn't directly support SDIO, which limits the transfer speeds between the reader and the esp to 1.3 MB/s, which may be too low to actually stream a .flac or .wav file off of. 
* I don't have a DAC on hand, and I'm not sure if I'll be able to purchase one for this project. Going this route means that I'll be stuck with 8-bit audio until I able to purchase one. 

Anyways, everything else I did:
* Tested options, looked at the DFplayer playback capabilities (horrid) and the SPI transfer speeds (hopefully not bad enough to limit the audio quality)
* Set up the lvgl library and started testing the OLED screen
* Desoldered an old component off of a 3.5 mm aux port and soldered on new pins. 
* Explored native audio playback for the esp32-c6

***
### __Day 2__
__10/17/2026__

Had a quick look at options for a DAC. The current contenders are:

| Component | Image | Functionality |
|-----------|-------|---------------|
| Adafruit PCM5102 I2S DAC with Line Level Output - 112dB SNR | ![Adafruit PCM5102 I2S DAC with Line Level Output.jpg](/img/Adafruit%20PCM5102%20I2S%20DAC%20with%20Line%20Level%20Output%20-%20112dB%20SNR%20.png) | Slightly less functionality compared to the PCM5122 based component, missing features such as sterio output and eq-customization. Is howerver more available and cheaper |
| Adafruit PCM5122 I2S DAC with Line Level Output - HW, I2C or SPI | ![Adafruit PCM5122 I2S DAC with Line Level Output - HW, I2C or SPI.jpg](/img/Adafruit%20PCM5122%20I2S%20DAC%20with%20Line%20Level%20Output%20-%20HW,%20I2C%20or%20SPI%20.jpg) | Comes in with quite a lot more funcitonality compared to the PCM502 based component. Key features include sterio audio, eq-customization and dynamic range control. |

Both components do however share some quality-of-life features, such as automatic file format detection and decoding, meaning that no actual decoding is necessary to be done directly on the esp, which is a nice thing to see looking at how it could have been a concern with more compressed file formats. 

As of today, this is what the player looks like:
![player status 10-17-2026.png](/img/player%20status%2010-17-2026.png)