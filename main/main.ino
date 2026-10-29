#include "AudioTools.h"                            // Decoder handler
#include "AudioTools/AudioCodecs/CodecFLACFoxen.h" // FLAC codec (Foxen)
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"  // MP3 codec
#include "AudioTools/AudioCodecs/CodecWAV.h"       // WAV codec
#include "AudioTools/AudioCodecs/MultiDecoder.h"   // picks the decoder per file
#include "AudioTools/Disk/AudioSourceSD.h"         // playlist from the SD card
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
#define DAC_MCK -1

I2SStream i2s;         // final output to the DAC
MP3DecoderHelix mp3;   // MP3 decoder
WAVDecoder wav;        // WAV decoder
FLACDecoderFoxen flac; // FLAC decoder
MultiDecoder multi;    // chooses mp3/wav/flac for each file

// All (non-hidden) files in the root folder, using the SPI bus set up below.
// ext = "" means "don't filter by extension".
AudioSourceSD source("/playlist1/", "", SD_CS, SPI);

// source -> multi decoder -> i2s (volume and fade stages are built in)
AudioPlayer player(source, i2s, multi);

float volume = 1.0;

// Called by the player whenever it switches to a new file
void onTrackChange(Stream *stream, void *ref) {
  Serial.print("Playing: ");
  Serial.println(source.toStr());
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // ---- SD card ----
  SPI.begin(SD_CLK, SD_SO, SD_SI, SD_CS);
  // Mounted here at 40 MHz. AudioSourceSD calls SD.begin() again later, which
  // (on the ESP32 core) should just report "already mounted".
  if (!SD.begin(SD_CS, SPI, 40000000)) {
    Serial.println("SD init failed - check wiring / card format (FAT32)");
    while (1)
      delay(1000);
  }

  // ---- I2S output ----
  auto cfg = i2s.defaultConfig(TX_MODE);
  cfg.buffer_count = 8;
  cfg.buffer_size = 1024;
  cfg.pin_bck = DAC_BCK;
  cfg.pin_ws = DAC_WSEL;
  cfg.pin_data = DAC_DIN;
  cfg.pin_mck = DAC_MCK;
  // sample rate / channels / bits are set automatically from the file's header
  i2s.begin(cfg);

  // ---- Decoders, selected by the file's extension ----
  multi.addDecoder(mp3, "audio/mpeg");
  multi.addDecoder(wav, "audio/vnd.wave");
  multi.addDecoder(flac, "audio/flac");

  // ---- Player ----
  player.setOnStreamChangeCallback(onTrackChange);
  player.setSilenceOnInactive(
      true);                // keep feeding zeros to the DAC while paused
  player.setVolume(volume); // 0.0 - 1.0
  if (!player.begin()) {    // starts with the first file on the card
    Serial.println(
        "Player start failed - no playable files in the root folder?");
    while (1)
      delay(1000);
  }
}

void loop() {
  player.copy(); // does the actual playing, must be called continuously

  // Serial input commands
  if (Serial.available()) {
    switch (Serial.read()) {
    case 'p':
      player.setActive(true);
      break; // resume
    case 's':
      player.setActive(false);
      break; // pause
    case 'n':
      player.next();
      break; // next track
    case 'b':
      player.previous();
      break; // previous track
    case '+':
      volume = min(1.0f, volume + 0.1f);
      player.setVolume(volume);
      break;
    case '-':
      volume = max(0.0f, volume - 0.1f);
      player.setVolume(volume);
      break;
    }
  }
}