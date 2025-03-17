#include "app.h"
#include "text.h"
#include "snakes.h"
#include "remote.h"
#include "cast.h"
#include <FastLED.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define DATA_PIN 42
#define ENCODER_A 9
#define ENCODER_B 10
#define BATTERY_PIN 1
#define BATTERY_BUTTON_PIN 7
#define LED1 35
#define LED2 36
#define LED3 37
#define LED4 38

CRGB leds[NUM_LEDS];

TaskHandle_t Task1;
TaskHandle_t Task2;

volatile int encoderPosition = 0;
volatile bool encoderChanged = false;
int buttonState = HIGH;
int lastButtonState = HIGH;

int mode = 0;

const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";
const char* mqtt_server = "YOUR_MQTT_BROKER_IP";

WiFiClient espClient;
PubSubClient client(espClient);

QueueHandle_t xQueue;

// Emcoder Interrupt Handler
void IRAM_ATTR handleEncoderA() {
  if (digitalRead(ENCODER_A) == HIGH) {
    if (digitalRead(ENCODER_B) == LOW) {
      encoderPosition++;
    } else {
      encoderPosition--;
    }
  } else {
    if (digitalRead(ENCODER_B) == LOW) {
      encoderPosition--;
    } else {
      encoderPosition++;
    }
  }
  encoderChanged = true;
}

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void reconnect() {
  while (!client.connected()) {
    String clientId = "ESP32-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  pinMode(ENCODER_A, INPUT);
  pinMode(ENCODER_B, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BATTERY_PIN, INPUT);
  pinMode(BATTERY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(ENCODER_A), handleEncoderA, CHANGE);

  setup_wifi();
  client.setServer(mqtt_server, 1883);

  xTaskCreatePinnedToCore(
    Task1code, /* Task function. */
    "Task1",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task1,    /* Task handle to keep track of created task */
    0);        /* pin task to core 0 */

  xTaskCreatePinnedToCore(
    Task2code, /* Task function. */
    "Task2",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task2,    /* Task handle to keep track of created task */
    1);        /* pin task to core 1 */
}

void loop() {
  // Empty. Tasks are running on different cores.
}

void Task1code(void * pvParameters) {
  for (;;) {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();

    int batteryButtonState = digitalRead(BATTERY_BUTTON_PIN);
    if (batteryButtonState == LOW) {
      showBatteryLevel();
      delay(3000); // Show battery level for 3 seconds
      clearBatteryLEDs();
    }
    delay(100);
  }
}

void Task2code(void * pvParameters) {
  textColor = CRGB::DarkViolet;
  startScrollText("REMOTE", TEXT_SPEED);
  for (;;) {
    mainMenu();
  }
}

void showBatteryLevel() {
  int batteryLevel = analogRead(BATTERY_PIN);
  int level = map(batteryLevel, 1600, 2400, 0, 4); // Assuming 3.0V is 0% and 4.2V is 100%
  Serial.println(batteryLevel);
  digitalWrite(LED1, level > 0 ? HIGH : LOW);
  digitalWrite(LED2, level > 1 ? HIGH : LOW);
  digitalWrite(LED3, level > 2 ? HIGH : LOW);
  digitalWrite(LED4, level > 3 ? HIGH : LOW);
}

void clearBatteryLEDs() {
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
}

void mainMenu(){
  if (encoderChanged) {
    encoderChanged = false;
    mode = encoderPosition % NUM_MODES;
    if (mode < 0) mode += NUM_MODES;
    switch (mode) {
      case 0:
        textColor = CRGB::DarkViolet;
        startScrollText("REMOTE", TEXT_SPEED);
        break;
      case 1:
        textColor = CRGB::ForestGreen;
        startScrollText("SNAKES", TEXT_SPEED);
        break;
      case 2:
        textColor = CRGB::MediumSlateBlue;
        startScrollText("CAST", TEXT_SPEED);
        break;
      default:
        break;
    }
  }
  updateScrollText(); // Update the scrolling text
  FastLED.show();

  buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && lastButtonState == HIGH) {
    lastButtonState = buttonState;
    delay(10);
    switch (mode) {
      case 0:
        startRemote();
        break;
      case 1:
        startSnakes();
        encoderChanged = true;
        break;
      case 2:
        startCast();
        break;
      default:
        break;
    }
  }
  lastButtonState = buttonState;
  delay(10);
}

