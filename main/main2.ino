#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include "AudioTools.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include "AudioTools/AudioCodecs/CodecFLAC.h"

// -------- Micro SD SPI breakout board --------
#define SD_CLK 0
#define SD_SO  1
#define SD_SI  10
#define SD_CS  11

// -------- PCM5122 I2S DAC --------
#define DAC_WSEL 23
#define DAC_DIN  22
#define DAC_BCK  21
#define DAC_MCK  20

I2SStream i2s;                                   // final output to the DAC
MP3DecoderHelix mp3;                             // MP3 decoder
WAVDecoder wav;                                  // WAV decoder
FLACDecoder flac;                                // 2. Declare FLAC decoder

EncodedAudioStream mp3Stream(&i2s, &mp3);       
EncodedAudioStream wavStream(&i2s, &wav);
EncodedAudioStream flacStream(&i2s, &flac);      // 3. Create FLAC audio stream pipeline
StreamCopy copier;                               // moves bytes from file to decoder

File audioFile;
bool playing = false;

bool hasExt(String name, const char *ext) {
  name.toLowerCase();
  return name.endsWith(ext);
}

bool isAudioFile(const String &name) {
  if (name.startsWith(".") || name.startsWith("_")) return false;  // skip macOS junk files
  // 4. Update filter to accept .flac files
  return hasExt(name, ".mp3") || hasExt(name, ".wav") || hasExt(name, ".flac");
}

// Returns the full path of the first supported audio file in the root folder
String findFirstAudioFile() {
  File root = SD.open("/");
  if (!root) return "";

  String found = "";
  while (true) {
    File entry = root.openNextFile();
    if (!entry) break;
    if (!entry.isDirectory() && isAudioFile(String(entry.name()))) {
      found = String(entry.path());
      entry.close();
      break;
    }
    entry.close();
  }
  root.close();
  return found;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);

  // ---- SD card ----
  SPI.begin(SD_CLK, SD_SO, SD_SI, SD_CS);
  if (!SD.begin(SD_CS)) {
    Serial.println("SD init failed - check wiring / card format (FAT32)");
    while (1) delay(1000);
  }

  // ---- Find a file ----
  String path = findFirstAudioFile();
  if (path == "") {
    Serial.println("No MP3, WAV, or FLAC files found in root folder");
    while (1) delay(1000);
  }
  Serial.print("Playing: ");
  Serial.println(path);

  audioFile = SD.open(path);
  if (!audioFile) {
    Serial.println("Could not open file");
    while (1) delay(1000);
  }

  // ---- I2S output ----
  auto cfg = i2s.defaultConfig(TX_MODE);
  cfg.pin_bck  = DAC_BCK;
  cfg.pin_ws   = DAC_WSEL;
  cfg.pin_data = DAC_DIN;
  cfg.pin_mck  = DAC_MCK;
  i2s.begin(cfg);

  // ---- Pick decoder by extension and start the copy chain ----
  if (hasExt(path, ".wav")) {
    wavStream.begin();
    copier.begin(wavStream, audioFile);
  } else if (hasExt(path, ".flac")) {
    flacStream.begin();
    copier.begin(flacStream, audioFile);
  } else {
    mp3Stream.begin();
    copier.begin(mp3Stream, audioFile);
  }
  playing = true;
}

void loop() {
  if (!playing) return;

  if (!copier.copy()) {
    Serial.println("Finished");
    audioFile.close();
    i2s.end();
    playing = false;
  }
}