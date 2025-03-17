#ifndef SNAKES_H
#define SNAKES_H

#include <FastLED.h>

class Snake {
    public:
        int currentX;
        int currentY;
        int tailX[NUM_LEDS];
        int tailY[NUM_LEDS];
        int snakeLength;
        int direction;
        int snakeColor;
    
        Snake() : currentX(2), currentY(0), snakeLength(2), direction(1), snakeColor(0) {
            tailX[0] = 1;
            tailY[0] = 0;
            tailX[1] = 0;
            tailY[1] = 0;
        }
    
        void moveSnake();
    };

void startSnakes();

#endif // SNAKES_H