#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <lvgl.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// ---------- Pins ----------
#define I2C_SDA    6
#define I2C_SCL    7
#define SD_SCK     19
#define SD_MOSI    18
#define SD_MISO    20
#define SD_CS      23

// ---------- Display ----------
#define SCREEN_W   128
#define SCREEN_H   64
#define OLED_ADDR  0x3C

// ---------- SD benchmark ----------
#define SD_FREQ     25000000      // try 4000000, 10000000, 20000000, 25000000
#define TEST_FILE   "/Nick-Drake-River-Man-Take-1-4th-January-1969.flac"   // change to your own file, e.g. "/song.mp3"
#define CHUNK_SIZE  4096
#define READ_RUNS   3

Adafruit_SSD1306 oled(SCREEN_W, SCREEN_H, &Wire, -1);

static uint16_t drawBuf[SCREEN_W * 16];   // 16-line partial buffer, RGB565
static uint8_t  buf[CHUNK_SIZE];

static lv_obj_t *infoLabel;
static lv_obj_t *uptimeLabel;

// ---------- LVGL -> OLED ----------
void flushCb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  uint16_t *px = (uint16_t *)px_map;
  for (int y = area->y1; y <= area->y2; y++) {
    for (int x = area->x1; x <= area->x2; x++) {
      uint16_t c = *px++;
      bool on = ((c >> 5) & 0x3F) > 31;          // threshold the green channel
      oled.drawPixel(x, y, on ? SSD1306_WHITE : SSD1306_BLACK);
    }
  }
  if (lv_display_flush_is_last(disp)) oled.display();
  lv_display_flush_ready(disp);
}

void updateUptimeCb(lv_timer_t *t) {
  lv_label_set_text_fmt(uptimeLabel, "Up: %lus", millis() / 1000);
}

// Set the info text and force LVGL to draw it now (needed because the
// benchmark blocks loop(), so lv_timer_handler() wouldn't run otherwise)
void showInfo(const char *fmt, ...) {
  char msg[96];
  va_list args;
  va_start(args, fmt);
  vsnprintf(msg, sizeof(msg), fmt, args);
  va_end(args);

  lv_label_set_text(infoLabel, msg);
  for (int i = 0; i < 3; i++) {
    lv_timer_handler();
    delay(5);
  }
}

// ---------- SD benchmark ----------
float toMBps(size_t bytes, uint32_t us) {
  return (bytes / 1048576.0f) / (us / 1000000.0f);
}

// Returns write speed in MB/s, or -1 if the file already existed
float createTestFile() {
  for (int i = 0; i < CHUNK_SIZE; i++) buf[i] = i & 0xFF;

  File f = SD.open(TEST_FILE, FILE_WRITE);
  if (!f) { Serial.println("Failed to create file"); return -1; }

  size_t total = 0;
  uint32_t t0 = micros();
  for (int i = 0; i < 256; i++) total += f.write(buf, CHUNK_SIZE);   // 1 MB
  f.close();
  uint32_t us = micros() - t0;

  float mbps = toMBps(total, us);
  Serial.printf("Write: %u bytes in %.3f s -> %.2f MB/s (%.1f Mbit/s)\n",
                (unsigned)total, us / 1e6f, mbps, mbps * 8.388608f);
  return mbps;
}

// Returns average read speed in MB/s, or -1 on failure
float readBenchmark() {
  float sum = 0;
  for (int run = 1; run <= READ_RUNS; run++) {
    File f = SD.open(TEST_FILE);
    if (!f) { Serial.println("Failed to open file"); return -1; }

    size_t total = 0;
    uint32_t t0 = micros();
    while (f.available()) total += f.read(buf, CHUNK_SIZE);
    uint32_t us = micros() - t0;
    f.close();

    float mbps = toMBps(total, us);
    Serial.printf("Read run %d: %u bytes in %.3f s -> %.2f MB/s (%.1f Mbit/s)\n",
                  run, (unsigned)total, us / 1e6f, mbps, mbps * 8.388608f);
    sum += mbps;
  }
  return sum / READ_RUNS;
}

void runSdTest() {
  showInfo("SD: mounting...");

  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, SPI, SD_FREQ)) {
    Serial.println("Card mount failed");
    showInfo("SD: mount failed");
    return;
  }

  uint64_t sizeMB = SD.cardSize() / (1024 * 1024);
  Serial.printf("Card size: %llu MB, SPI clock: %d MHz\n", sizeMB, SD_FREQ / 1000000);

  float writeMBps = -1;
  if (!SD.exists(TEST_FILE)) {
    showInfo("SD %lluMB\nWriting 1MB...", sizeMB);
    writeMBps = createTestFile();
  }

  showInfo("SD %lluMB\nReading...", sizeMB);
  float readMBps = readBenchmark();

  if (readMBps < 0) {
    showInfo("SD %lluMB\nRead failed", sizeMB);
    return;
  }

  if (writeMBps >= 0) {
    showInfo("SD %lluMB @%dMHz\nRead:  %.2f MB/s\nWrite: %.2f MB/s",
             sizeMB, SD_FREQ / 1000000, readMBps, writeMBps);
  } else {
    showInfo("SD %lluMB @%dMHz\nRead:  %.2f MB/s\n(%.1f Mbit/s)",
             sizeMB, SD_FREQ / 1000000, readMBps, readMBps * 8.388608f);
  }
}

// ---------- Setup / loop ----------
void setup() {
  Serial.begin(115200);
  delay(1000);

  // OLED
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);
  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 not found - check wiring/address");
    while (true) delay(1000);
  }
  oled.clearDisplay();
  oled.display();

  // LVGL
  lv_init();
  lv_tick_set_cb([]() -> uint32_t { return millis(); });

  lv_display_t *disp = lv_display_create(SCREEN_W, SCREEN_H);
  lv_display_set_flush_cb(disp, flushCb);
  lv_display_set_buffers(disp, drawBuf, NULL, sizeof(drawBuf),
                         LV_DISPLAY_RENDER_MODE_PARTIAL);

  lv_obj_t *scr = lv_screen_active();
  lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
  lv_obj_set_style_text_color(scr, lv_color_white(), 0);

#if LV_FONT_UNSCII_8
  lv_obj_set_style_text_font(scr, &lv_font_unscii_8, 0);   // crisp pixel font
#endif

  infoLabel = lv_label_create(scr);
  lv_label_set_text(infoLabel, "Starting...");
  lv_obj_align(infoLabel, LV_ALIGN_TOP_LEFT, 0, 0);

  uptimeLabel = lv_label_create(scr);
  lv_label_set_text(uptimeLabel, "Up: 0s");
  lv_obj_align(uptimeLabel, LV_ALIGN_BOTTOM_RIGHT, 0, 0);

  lv_timer_create(updateUptimeCb, 1000, NULL);

  // SD benchmark (results shown on screen and Serial)
  runSdTest();
}

void loop() {
  lv_timer_handler();
  delay(5);
}