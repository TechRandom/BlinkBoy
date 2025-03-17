#include "text.h"
#include "app.h"
#include "cast.h"

bool running = true;
char hexByte[3];

void castCallback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, "lmcshd/data") == 0) {
    client.publish("cast/ack", "ack");
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].r = *payload++;
      leds[i].g = *payload++;
      leds[i].b = *payload++;
    }
    FastLED.show();
  }
}

void startCast() {
  client.setBufferSize(1024);
  client.setCallback(castCallback);
  client.subscribe("lmcshd/data");
  client.publish("cast/ack", "ack");
  running = true;
  while (running) {
    buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == LOW && lastButtonState == HIGH) {
      lastButtonState = buttonState;
      running = false;
      delay(10);
    }
    lastButtonState = buttonState;
    delay(10);
  }
  client.unsubscribe("lmcshd/data");
  return;
}