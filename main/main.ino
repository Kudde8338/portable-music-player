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

float volume = 1.0;

enum AudioCommand {
  CMD_PLAY,
  CMD_PAUSE,
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

  if (!SD.begin(SD_CS, SPI, 40000000)) {  // Mounted here at 40 MHz.
    Serial.println("SD init failed - check wiring / card format (FAT32)");
    while (1)
      delay(1000);
  }

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
        case CMD_PLAY:
          player.setActive(true);
          break;

        case CMD_PAUSE:
          player.setActive(false);
          break;

        case CMD_NEXT:
          player.next();
          break;

        case CMD_PREVIOUS:
          player.previous();
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
    taskYIELD();
  }
}

void controlTask(void *parameter) {
  while (true) {
    if (Serial.available()) {

      AudioCommand command;
      bool validCommand = true;

      switch (Serial.read()) {
        default:
          validCommand = false;
          break;
          
        case 'p':
          command = CMD_PLAY;
          break;  // resume
        case 's':
          command = CMD_PAUSE;
          break;  // pause
        case 'n':
          command = CMD_NEXT;
          break;  // next track
        case 'b':
          command = CMD_PREVIOUS;
          break;  // previous track
        case '+':
          command = CMD_VOLUME_UP;
          break;
        case '-':
          command = CMD_VOLUME_DOWN;
          break;
      }

      if (validCommand) {
        if (xQueueSend(audioQueue, &command, 0) != pdTRUE) {
          Serial.println("Queue is full. Couldn't send command!");
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}