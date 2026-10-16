# The absolutely coolest mp3 player ever!

Write a short description here

## How i did it 

As of this moment a DFPlayer mini is used to convert the MP3 (or similar files) directly to an audio output. This method, however, isn't all that efficient at getting the most out of the audio quality. The alternative (which is more favourable, but harder and more expensive to pull off) would be to use a microSD breakout card and convert the MP3 files (or similar) to audio signals directly on the ESP, and then running this through a DAC to get an audio output. Running this method without a DAC would mean only having 8-bit audio, compared to the 24-bit audio you get from the MP3-TF-16P SDcard module (as a reference: lossless audio requires )

### Components
* ESP32-C6-DEV-KIT-NX (With presoldered headers)
* MP3-TF-16P SDcard module
* 0.96" OLED 128x64px I2C white

### Libraries
The program makes use of the following libraries:
* DFRobotDFPlayerMini by *DFRobot*