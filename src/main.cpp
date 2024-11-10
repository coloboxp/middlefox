/**
 * Auto-capture images to SD card every 10 seconds
 * with consecutive filenames
 */

#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
#define LED_PIN 21

#include <Arduino.h>
#include <camera_pins.h>
#include <eloquent_esp32cam.h>
#include <SD.h>
#include <esp_log.h>

using namespace eloq;

static const char *TAG = "Camera"; // Tag for logging

// Time tracking
unsigned long lastCapture = 0;
const unsigned long CAPTURE_INTERVAL = 5000; // 5 seconds in milliseconds
int imageCount = 0;

int getNextImageCount()
{
  File root = SD.open("/");
  int maxCount = 0;

  while (true)
  {
    File entry = root.openNextFile();
    if (!entry)
      break;

    String fileName = String(entry.name());
    if (fileName.startsWith("picture") && fileName.endsWith(".jpg"))
    {
      // Extract number between "picture" and ".jpg"
      String numStr = fileName.substring(7, fileName.length() - 4);
      int num = numStr.toInt();
      if (num > maxCount)
        maxCount = num;
    }
    entry.close();
  }

  root.close();
  return maxCount + 1;
}

bool convert_rgb565_to_jpeg(const uint8_t* rgb565_data, int width, int height, uint8_t** jpg_buf_out, size_t* jpg_len_out) {
    if (!rgb565_data || !jpg_buf_out || !jpg_len_out) {
        ESP_LOGE(TAG, "Invalid parameters");
        return false;
    }

    camera_fb_t fb = {
        .buf = (uint8_t*)rgb565_data,
        .len = width * height * 2,  // RGB565 is 2 bytes per pixel
        .width = width,
        .height = height,
        .format = PIXFORMAT_RGB565
    };

    bool success = frame2jpg(&fb, 100, jpg_buf_out, jpg_len_out);  // Use maximum quality (100)
    
    if (success && *jpg_len_out > 0) {
        ESP_LOGI(TAG, "JPEG conversion successful, size: %d", *jpg_len_out);
        return true;
    }

    ESP_LOGE(TAG, "JPEG conversion failed");
    return false;
}

void setup()
{
  delay(3000);
  Serial.begin(115200);
  ESP_LOGI(TAG, "___AUTO CAPTURE TO SD CARD___");

  camera.pinout.pins.d0 = Y2_GPIO_NUM;
  camera.pinout.pins.d1 = Y3_GPIO_NUM;
  camera.pinout.pins.d2 = Y4_GPIO_NUM;
  camera.pinout.pins.d3 = Y5_GPIO_NUM;
  camera.pinout.pins.d4 = Y6_GPIO_NUM;
  camera.pinout.pins.d5 = Y7_GPIO_NUM;
  camera.pinout.pins.d6 = Y8_GPIO_NUM;
  camera.pinout.pins.d7 = Y9_GPIO_NUM;
  camera.pinout.pins.xclk = XCLK_GPIO_NUM;
  camera.pinout.pins.pclk = PCLK_GPIO_NUM;
  camera.pinout.pins.vsync = VSYNC_GPIO_NUM;
  camera.pinout.pins.href = HREF_GPIO_NUM;
  camera.pinout.pins.sccb_sda = SIOD_GPIO_NUM;
  camera.pinout.pins.sccb_scl = SIOC_GPIO_NUM;
  camera.pinout.pins.pwdn = PWDN_GPIO_NUM;
  camera.pinout.pins.reset = RESET_GPIO_NUM;

  camera.pixformat.rgb565();
  camera.resolution.face();
  camera.quality.best();
  camera.sensor.enableAutomaticExposureControl();
  camera.sensor.enableAutomaticGainControl();
  camera.sensor.enableAutomaticWhiteBalance();
  camera.sensor.enableAutomaticWhiteBalanceGain();

  // init camera
  while (!camera.begin().isOk())
  {
    ESP_LOGE(TAG, "%s", camera.exception.toString().c_str());
    delay(1000);
  }
  ESP_LOGI(TAG, "Camera OK");

  // Initialize SD card
  if (!SD.begin(21))
  { // Pin 21 for XIAO ESP32S3
    ESP_LOGE(TAG, "SD Card Mount Failed");
    return;
  }

  // Get the next available image count
  imageCount = getNextImageCount();
  ESP_LOGI(TAG, "Starting with image count: %d", imageCount);

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE)
  {
    ESP_LOGE(TAG, "No SD card attached");
    return;
  }

  ESP_LOGI(TAG, "SD card OK");
  ESP_LOGI(TAG, "Starting auto-capture every 5 seconds...");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Ensure LED is off at start
}

void loop()
{
  unsigned long currentTime = millis();

  if (currentTime - lastCapture >= CAPTURE_INTERVAL)
  {
    digitalWrite(LED_PIN, HIGH);

    if (!camera.capture().isOk())
    {
      ESP_LOGE(TAG, "%s", camera.exception.toString().c_str());
      digitalWrite(LED_PIN, LOW);
      lastCapture = currentTime;
      return;
    }

    // Save RGB565 raw data
    String rgb_path = "/picture" + String(imageCount) + ".rgb";
    File rgb_file = SD.open(rgb_path.c_str(), FILE_WRITE);
    if (rgb_file)
    {
      rgb_file.write(camera.frame->buf, camera.frame->len);
      rgb_file.close();
      ESP_LOGI(TAG, "Saved RGB file: %s", rgb_path.c_str());
    }

    // Convert and save highest quality JPEG
    uint8_t *jpg_buf = NULL;
    size_t jpg_len = 0;
    bool converted = convert_rgb565_to_jpeg(camera.frame->buf, camera.frame->width, camera.frame->height, &jpg_buf, &jpg_len); // 0 = highest quality

    if (converted)
    {
      String jpg_path = "/picture" + String(imageCount) + ".jpg";
      File jpg_file = SD.open(jpg_path.c_str(), FILE_WRITE);
      if (jpg_file)
      {
        jpg_file.write(jpg_buf, jpg_len);
        jpg_file.close();
        ESP_LOGI(TAG, "Saved JPG file: %s", jpg_path.c_str());
      }
      free(jpg_buf);
      imageCount++;
    }

    digitalWrite(LED_PIN, LOW);
    lastCapture = currentTime;
  }
}