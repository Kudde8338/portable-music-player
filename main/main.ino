#include "AudioTools.h"
#include "AudioTools/AudioCodecs/CodecFLAC.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include <FS.h>
#include <SD.h>
#include <SPI.h>

// -------- Micro SD SPI breakout board --------
#define SD_CLK 0
#define SD_SO 1
#define SD_SI 10
#define SD_CS 11

// -------- PCM5122 I2S DAC --------
#define DAC_WSEL 23
#define DAC_DIN 22
#define DAC_BCK 21
#define DAC_MCK 20

I2SStream i2s; // final output to the DAC

MP3DecoderHelix mp3; // MP3 decoder
WAVDecoder wav;      // WAV decoder
FLACDecoder flac;    // FLAC decoder

EncodedAudioStream mp3Stream(&i2s, &mp3);
EncodedAudioStream wavStream(&i2s, &wav);
EncodedAudioStream flacStream(&i2s, &flac);
StreamCopy copier;

File audioFile;
bool playing = false;

class modes {

  // Generally how the class system funcitons
  // A function is called, in "play_all" functions the work looks as follows:
  //        - A path is given to the specified playlist (csv) from which the
  //        player is to play from
  //        - The program will decode the file listed in the specified path in
  //        the csv
  //
  // In "play_one" functions the work looks as follows:
  //        - Get the immediate path of the audio file, no csv instructions
  //        - The program will decode the directly specified file under that
  //        path

public:
  void play_all_straight(String path) {}

  void play_all_shuffle(String path) {}

  void play_one_once(String path) {}

  void play_one_repeat(String path, const int times) {}
};

// -------- Filter out unsupported file formats --------
bool hasExt(String name, const char *ext) {
  name.toLowerCase();
  return name.endsWith(ext);
}
bool isAudioFile(const String &name) {
  if (name.startsWith(".") || name.startsWith("_"))
    return false;
  return hasExt(name, ".mp3") || hasExt(name, ".wav") || hasExt(name, ".flac");
}

// -------- Initialize components  --------
void Initialize_SPI() {
  SPI.begin(SD_CLK, SD_SO, SD_SI, SD_CS);
  if (!SD.begin(SD_CS)) {
    Serial.println("SD init failed - check wiring / card format (FAT32)");
    while (1)
      delay(1000);
  }
}
void Initialize_I2S() {
  auto cfg = i2s.defaultConfig(TX_MODE);
  cfg.pin_bck = DAC_BCK;
  cfg.pin_ws = DAC_WSEL;
  cfg.pin_data = DAC_DIN;
  cfg.pin_mck = DAC_MCK;

  i2s.begin(cfg);
  if (i2s.begin(cfg)) {
    Serial.println("I2S init failed - check wiring");
    while (1)
      delay(1);
  }
}

// -------- On startup --------
void startup() {
  // Set the current mode of the player
  // Find an appropriate file to play
  // Play the prediscribed file, based on the instructions written in the mode
}

void Setup(void) {
  Serial.begin(115200);
  delay(1000);

  // Initialize components
  Initialize_SPI();
  Initialize_I2S();

  delay(1000);
}