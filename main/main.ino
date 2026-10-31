// -------- Standard libraries -------- 
#include <FS.h>
#include <SD.h>
#include <SPI.h>

// -------- Audiotools --------
#include "AudioTools.h"                            // Decoder handler
#include "AudioTools/AudioCodecs/CodecFLACFoxen.h" // FLAC codec (Foxen)
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"  // MP3 codec
#include "AudioTools/AudioCodecs/CodecWAV.h"       // WAV codec
#include "AudioTools/AudioCodecs/MultiDecoder.h"   // picks the decoder per file
#include "AudioTools/Disk/AudioSourceSD.h"         // playlist from the SD card

// -------- Visuals --------
#include <U8g2lib.h>

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

// -------- I2C OLED --------
#define OLED_SCL 7
#define OLED_SDA 6
#define OLED_width 128 // Dimensions
#define OLED_height 64 //

// -------- Decoder definitions --------
I2SStream i2s;         // final output to the DAC
MP3DecoderHelix mp3;   // MP3 decoder
WAVDecoder wav;        // WAV decoder
FLACDecoderFoxen flac; // FLAC decoder
MultiDecoder multi;    // chooses mp3/wav/flac for each file

// -------- Player variable setup --------
// All (non-hidden) files in the root folder, using SPI
// ext = "" means "don't filter by extension".
AudioSourceSD source("/Speciella vibbar", "", SD_CS, SPI);

// source -> multi decoder -> i2s (volume and fade stages are built in)
AudioPlayer player(source, i2s, multi);

float volume = 1.0;

// -------- I2C OLED definition --------
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/OLED_SCL, /* data=*/OLED_SDA, /* reset=*/U8X8_PIN_NONE); // ESP32 Thing, pure SW emulated I2C

// Called by the player whenever it switches to a new file
void onTrackChange(Stream *stream, void *ref) {
  Serial.print("Playing: ");
  Serial.println(source.toStr());
  drawPlaying_screen(source.toStr());
}

void drawPlaying_screen(const String& input) {

  // Strip the extension
  String name = input;
  int dot = name.lastIndexOf('.');
  if (dot >= 0) name.remove(dot);

  // Split on " - "
  String titleStr, artistStr;
  int sep = name.indexOf(" - ");
  if (sep >= 0) {
    titleStr  = name.substring(0, sep);
    artistStr = name.substring(sep + 3);
  } else {
    titleStr = name;  // no separator found
  }

  // Valid as long as titleStr/artistStr are in scope
  const char* title  = titleStr.c_str();
  const char* artist = artistStr.c_str();

  u8g2.clearBuffer(); // clear the internal memory

  u8g2.setFontMode(1);
  u8g2.setBitmapMode(1);

  // Song title
  u8g2.setFont(u8g2_font_timR14_tf);
  u8g2.drawUTF8(3, 23, title);
  // Credited artist
  u8g2.setFont(u8g2_font_timR10_tr);
  u8g2.drawStr(5, 42, artist);

  u8g2.sendBuffer(); // transfer internal memory to the display
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // ---- I2C OLED ----
  u8g2.begin();

  // ---- SD card ----
  SPI.begin(SD_CLK, SD_SO, SD_SI, SD_CS);

  if (!SD.begin(SD_CS, SPI, 40000000)) {   // Mounted here at 40 MHz.
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
  player.setSilenceOnInactive(true);                // keep feeding zeros to the DAC while paused
  player.setVolume(volume);  // 0.0 - 1.0
  if (!player.begin()) {// starts with the first file on the card
    Serial.println("Player start failed - no playable files in the root folder?");
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