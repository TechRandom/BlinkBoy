#ifndef APP_H
#define APP_H

#include <FastLED.h>
#include <PubSubClient.h>

#define MATRIX_WIDTH 11
#define MATRIX_HEIGHT 11
#define NUM_LEDS (MATRIX_WIDTH * MATRIX_HEIGHT)
#define BRIGHTNESS 32
#define BUTTON_PIN 8
#define NUM_MODES 4

extern CRGB leds[];
extern volatile bool encoderChanged;
extern volatile int encoderPosition;
extern int buttonState;
extern int lastButtonState;

extern PubSubClient client;

int XY(int x, int y);

#endif // APP_H