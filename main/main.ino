// -------- Standard libraries --------
#include <FS.h>
#include <SD.h>
#include <SPI.h>

// -------- Audiotools --------

#include "AudioTools.h"                             // Decoder handler
#include "AudioTools/AudioCodecs/CodecFLACFoxen.h"  // FLAC codec (Foxen)
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"   // MP3 codec
#include "AudioTools/AudioCodecs/CodecWAV.h"        // WAV codec
#include "AudioTools/AudioCodecs/MultiDecoder.h"    // picks the decoder per file
#include "AudioTools/Disk/AudioSourceSD.h"          // playlist from the SD card

// -------- Micro SD SPI breakout board --------
#define SD_CLK 18
#define SD_SO 19
#define SD_SI 23
#define SD_CS 5

// -------- PCM5122 I2S DAC --------
#define DAC_WSEL 25
#define DAC_DIN 26
#define DAC_BCK 27
#define DAC_MCK -1

// -------- Buttons --------
#define BUTTON_PLAYBACK 13
#define BUTTON_NEXT 14
#define BUTTON_PREVIOUS 32
#define BUTTON_VOLUME_UP 33
#define BUTTON_VOLUME_DOWN 16

// -------- Decoder definitions --------
I2SStream i2s;          // final output to the DAC
MP3DecoderHelix mp3;    // MP3 decoder
WAVDecoder wav;         // WAV decoder
FLACDecoderFoxen flac;  // FLAC decoder
MultiDecoder multi;     // chooses mp3/wav/flac for each file

// -------- Player variable setup --------
// All (non-hidden) files in the specified folder, using SPI
// ext = "" means "don't filter by extension".
AudioSourceSD source("/music", "", SD_CS, SPI);

// source -> multi decoder -> i2s (volume and fade stages are built in)
AudioPlayer player(source, i2s, multi);

float volume = 0.5;
int trackCount = 0;
volatile bool userPaused = false;

enum AudioCommand {
  CMD_PLAYBACK,
  CMD_NEXT,
  CMD_PREVIOUS,
  CMD_VOLUME_UP,
  CMD_VOLUME_DOWN
};

QueueHandle_t audioQueue;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // ---- SD card ----
  SPI.begin(SD_CLK, SD_SO, SD_SI, SD_CS);

  while (!SD.begin(SD_CS, SPI, 40000000)) {  // Mounted here at 40 MHz.
    Serial.println("SD init failed - check wiring / card format (FAT32)");
    Serial.println("Trying again in 5 seconds");
    delay(5000);
  }
  trackCount = countTracks("/music");
  Serial.printf("Number of songs: %d\n", trackCount);

  delay(1000);

  // ---- Buttons ----
  pinMode(BUTTON_PLAYBACK, INPUT_PULLUP);
  pinMode(BUTTON_NEXT, INPUT_PULLUP);
  pinMode(BUTTON_PREVIOUS, INPUT_PULLUP);
  pinMode(BUTTON_VOLUME_UP, INPUT_PULLUP);
  pinMode(BUTTON_VOLUME_DOWN, INPUT_PULLUP);

  // ---- I2S output ----
  auto cfg = i2s.defaultConfig(TX_MODE);
  cfg.buffer_count = 8;
  cfg.buffer_size = 1023;
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
  player.setAutoFade(false);
  player.setSilenceOnInactive(true);  // keep feeding zeros to the DAC while paused
  player.setVolume(volume);           // 0.0 - 1.0
  if (!player.begin()) {              // starts with the first file on the card
    Serial.println("Player start failed - no playable files in the root folder?");
    while (1)
      delay(1000);
  }

  audioQueue = xQueueCreate(10, sizeof(AudioCommand));

  if (audioQueue == NULL) {
    Serial.println("Queue creation failed!");
    while (true) {
      delay(1000);
    }
  }

  xTaskCreatePinnedToCore(audioTask, "AudioTask", 8192, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(controlTask, "ControlTask", 4096, NULL, 1, NULL, 0);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}

void audioTask(void *parameter) {
  AudioCommand command;


  while (true) {
    while (xQueueReceive(audioQueue, &command, 0) == pdTRUE) {
      switch (command) {
        case CMD_PLAYBACK:
          if (player.isActive()) {
            player.setActive(false);
            userPaused = true;
          } else {
            player.setActive(true);
            userPaused = false;
          }
          break;

        case CMD_NEXT:
          userPaused = false;
          if (source.index() == trackCount - 1) {
            player.setIndex(0);
          } else {
            player.next();
          }
          player.setActive(true);
          break;

        case CMD_PREVIOUS:
          userPaused = false;
          if (source.index() == 0) {
            player.setIndex(trackCount - 1);
          } else {
            player.previous();
          }
          player.setActive(true);
          break;

        case CMD_VOLUME_UP:
          volume = min(1.0f, volume + 0.1f);
          player.setVolume(volume);
          break;

        case CMD_VOLUME_DOWN:
          volume = max(0.0f, volume - 0.1f);
          player.setVolume(volume);
          break;
      }
    }

    player.copy();
    if (!player.isActive() && !userPaused && trackCount > 0) {
      player.setIndex(0);
      userPaused = true;
      player.setActive(false);
    }
    taskYIELD();
  }
}

void controlTask(void *parameter) {
  bool lastPlayback = false, lastNext = false, lastPrevious = false, lastVolumeUp = false, lastVolumeDown = false;
  while (true) {
    if (pressed(BUTTON_PLAYBACK, lastPlayback)) {
      sendCmd(CMD_PLAYBACK);
    }
    if (pressed(BUTTON_NEXT, lastNext)) {
      sendCmd(CMD_NEXT);
    }
    if (pressed(BUTTON_PREVIOUS, lastPrevious)) {
      sendCmd(CMD_PREVIOUS);
    }
    if (pressed(BUTTON_VOLUME_UP, lastVolumeUp)) {
      sendCmd(CMD_VOLUME_UP);
    }
    if (pressed(BUTTON_VOLUME_DOWN, lastVolumeDown)) {
      sendCmd(CMD_VOLUME_DOWN);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

int countTracks(const char *path) {
  int n = 0;
  File dir = SD.open(path);
  if (!dir) return 0;
  for (File f = dir.openNextFile(); f; f = dir.openNextFile()) {
    if (!f.isDirectory() && f.name()[0] != '.') n++;
  }
  dir.close();
  return n;
}

bool pressed(int pin, bool &last) {
  bool now = digitalRead(pin) == LOW;
  bool edge = now && !last;
  last = now;
  return edge;
}

void sendCmd(AudioCommand c) {
  if (xQueueSend(audioQueue, &c, 0) != pdTRUE) {
    Serial.println("Queue is full. Couldn't send command!");
  }
}
