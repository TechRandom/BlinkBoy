#include "app.h"
#include "text.h"
#include "remote.h"

#define NUM_DEVICES 2

uint8_t device = 0;
const int centerX = 5;
const int centerY = 5;
const int wheelRadius = 3;
uint8_t hue = 0;

void drawColorWheel() {
  FastLED.clear();
  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      int dx = x - centerX;
      int dy = y - centerY;
      if (dx * dx + dy * dy <= wheelRadius * wheelRadius) {
        leds[XY(x, y)] = CHSV(hue, 255, 255);
      }
    }
  }
  FastLED.show();
}

void setColor(String dev){
  hue = 0;
  drawColorWheel();
  encoderPosition = 0;
  while(1){
    if (encoderChanged) {
      // Change color of circle
      hue = encoderPosition % 64;
      if (hue < 0) hue += 64;
      hue *= 4;
      drawColorWheel();
    }
    buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == LOW && lastButtonState == HIGH) {
      lastButtonState = buttonState;
      // Get Color of center pixel. It will be in dec so 
      uint8_t r, g, b;
      r = leds[60].r;
      g = leds[60].g; 
      b = leds[60].b;  
      char s[8]; 
      sprintf(s, "#%02x%02x%02x", r, g, b);
      const char* payload = s;
      if (dev == "desk"){
        client.publish("wled/desk/col", payload);
        client.publish("wled/desk", "255");
      }
      if (dev == "shelf"){
        client.publish("wled/shelf/col", payload);
        client.publish("wled/shelf", "255");
      }
      delay(10);
      return;
    }
    lastButtonState = buttonState;
    delay(10);
  }
}

void startRemote(){
  textColor = CRGB::Salmon;
  encoderPosition = 0;
  startScrollText("DESK", TEXT_SPEED);
  while(1){
    if (encoderChanged) {
      encoderChanged = false;
      device = encoderPosition % NUM_DEVICES;
      if (device < 0) device += NUM_DEVICES;
      switch (device) {
        case 0:
          textColor = CRGB::Salmon;
          startScrollText("DESK", TEXT_SPEED);
          break;
        case 1:
          textColor = CRGB::RoyalBlue;
          startScrollText("SHELF", TEXT_SPEED);
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
      switch (device) {
        case 0:
          setColor("desk");
          break;
        case 1:
          setColor("shelf");
          break;
        default:
          break;
      }
    }
    lastButtonState = buttonState;
    delay(10);
  }
}

