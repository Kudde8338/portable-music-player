#include "AudioTools.h"                            // Decoder handler
#include "AudioTools/AudioCodecs/CodecFLACFoxen.h" // FLAC codec (Foxen)
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"  // MP3 codec
#include "AudioTools/AudioCodecs/CodecWAV.h"       // WAV codec
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

I2SStream i2s;         // final output to the DAC
MP3DecoderHelix mp3;   // MP3 decoder
WAVDecoder wav;        // WAV decoder
FLACDecoderFoxen flac; // FLAC decoder

EncodedAudioStream mp3Stream(&i2s, &mp3); // file data -> decoded PCM -> I2S
EncodedAudioStream wavStream(&i2s, &wav);
EncodedAudioStream flacStream(&i2s, &flac);

StreamCopy copier; // moves bytes from file to decoder

File audioFile;
bool playing = false;

bool hasExt(String name, const char *ext) {
  name.toLowerCase();
  return name.endsWith(ext);
}

bool isAudioFile(const String &name) {
  if (name.startsWith(".") || name.startsWith("_"))
    return false; // skip macOS junk files
  return hasExt(name, ".flac");
}

// Returns the full path of the first MP3/WAV/FLAC in the root folder ("" if
// none)
String findFirstAudioFile() {
  File root = SD.open("/");
  if (!root)
    return "";

  String found = "";
  while (true) {
    File entry = root.openNextFile();
    if (!entry)
      break;
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

  // ---- SD card ----
  SPI.begin(SD_CLK, SD_SO, SD_SI, SD_CS);
  if (!SD.begin(SD_CS, SPI, 40000000)) {
    Serial.println("SD init failed - check wiring / card format (FAT32)");
    while (1)
      delay(1000);
  }

  // ---- Find a file ----
  String path = findFirstAudioFile();
  if (path == "") {
    Serial.println("No MP3/WAV/FLAC files found in root folder");
    while (1)
      delay(1000);
  }
  Serial.print("Playing: ");
  Serial.println(path);

  audioFile = SD.open("/Nick-Drake-River-Man-Take-1-4th-January-1969.flac");
  if (!audioFile) {
    Serial.println("Could not open file");
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
  if (!playing)
    return;

  // copy() moves one chunk from the file into the decoder; returns 0 at end of
  // file
  if (!copier.copy()) {
    Serial.println("Finished");
    audioFile.close();
    i2s.end();
    playing = false;
  }
}