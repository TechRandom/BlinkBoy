#include "HWCDC.h"
#include "text.h"
#include "app.h"
#include "snakes.h"
#include <PubSubClient.h>

#define MAX_PLAYERS 4
#define PAYLOAD_SIZE 10

extern PubSubClient client;

Snake players[MAX_PLAYERS];
int numPlayers = 0;
int myNumber;
int foodX;
int foodY;
CRGB foodColor = CRGB::White;
bool encoderHandled = false;
bool gameStarted = false;
unsigned long lastMoveTime = 0;
const unsigned int moveInterval = 250;
bool updateTitle = false;

CRGB colors[10] = {CRGB::Red, CRGB::Green, CRGB::Blue, CRGB::Yellow, CRGB::Cyan, CRGB::Magenta, CRGB::Orange, CRGB::Purple, CRGB::White, CRGB::Pink};

void spawnFood() {
  while (true) {
    foodX = random(MATRIX_WIDTH);
    foodY = random(MATRIX_HEIGHT);
    bool onSnake = false;
    for (int i = 0; i < numPlayers; i++) {
      for (int j = 0; j < players[i].snakeLength; j++) {
        if (foodX == players[i].tailX[j] && foodY == players[i].tailY[j]) {
          onSnake = true;
          break;
        }
      }
      if (foodX == players[i].currentX && foodY == players[i].currentY) {
        onSnake = true;
      }
    }
    if (!onSnake) {
      break;
    }
  }
  //broadcast food location
  byte payload[2] = { (byte)foodX, (byte)foodY };
  client.publish("snake/food", payload, 2);
}

void resetGame() {
  numPlayers = 1;

  for (int i = 0; i < MAX_PLAYERS; i++){
    players[i] = Snake();
  }

  // Initialize snake positions and directions
  players[0].currentX = 2;
  players[0].currentY = 0;
  players[0].tailX[0] = 1;
  players[0].tailX[1] = 0;
  players[0].tailY[0] = 0;
  players[0].tailY[1] = 0;
  players[0].direction = 1;

  players[1].currentX = 8;
  players[1].currentY = 10;
  players[1].tailX[0] = 9;
  players[1].tailX[1] = 10;
  players[1].tailY[0] = 10;
  players[1].tailY[1] = 10;
  players[1].direction = 3;

  players[2].currentX = 10;
  players[2].currentY = 2;
  players[2].tailX[0] = 10;
  players[2].tailX[1] = 10;
  players[2].tailY[0] = 1;
  players[2].tailY[1] = 0;
  players[2].direction = 0;

  players[3].currentX = 0;
  players[3].currentY = 8;
  players[3].tailX[0] = 0;
  players[3].tailX[1] = 0;
  players[3].tailY[0] = 9;
  players[3].tailY[1] = 10;
  players[3].direction = 2;
}

void updateLEDs() {
  FastLED.clear();
  for (int i = 0; i < numPlayers; i++) {
    if(players[i].snakeLength != 0){
      leds[XY(players[i].currentX, players[i].currentY)] = colors[players[i].snakeColor];
      for (int j = 0; j < players[i].snakeLength; j++) {
          leds[XY(players[i].tailX[j], players[i].tailY[j])] = colors[players[i].snakeColor];
      }
    }
  }
  leds[XY(foodX, foodY)] = foodColor;
  FastLED.show();
}

void showStartScreen() {
  for (int i = 0; i < numPlayers; i++){
    leds[XY(players[i].currentX, players[i].currentY)] = colors[players[i].snakeColor];
    leds[XY(players[i].tailX[0], players[i].tailY[0])] = colors[players[i].snakeColor];
    leds[XY(players[i].tailX[1], players[i].tailY[1])] = colors[players[i].snakeColor];
    FastLED.show();
  }
}

void gameOver() {
    for (int brightness = BRIGHTNESS; brightness >= 0; brightness--) {
        FastLED.setBrightness(brightness);
        FastLED.show();
        delay(10);
    }
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();
    delay(500);
    gameStarted = false;
}

void Snake::moveSnake() {
  int tempX = currentX;
  int tempY = currentY;

  switch (direction) {
    case 0: // Up
      currentY = (currentY + 1) % MATRIX_HEIGHT;
      break;
    case 1: // Right
      currentX = (currentX + 1) % MATRIX_WIDTH;
      break;
    case 2: // Down
      currentY = (currentY - 1 + MATRIX_HEIGHT) % MATRIX_HEIGHT;
      break;
    case 3: // Left
      currentX = (currentX - 1 + MATRIX_WIDTH) % MATRIX_WIDTH;
      break;
  }

  // Check if the snake eats the food
  if (currentX == foodX && currentY == foodY) {
    snakeLength++;
    spawnFood();
  }

  // Update tail positions
  for (int i = snakeLength - 1; i > 0; i--) {
    tailX[i] = tailX[i - 1];
    tailY[i] = tailY[i - 1];
  }
  tailX[0] = tempX;
  tailY[0] = tempY;
}

void gameLoop() {
  unsigned long currentTime = millis();
  if (currentTime - lastMoveTime >= moveInterval) {
    lastMoveTime = currentTime;
    for (int i = 0; i < numPlayers; i++) {
      players[i].moveSnake();
    }
    // Check if the snake eats itself
    for (int j = 0; j < numPlayers; j++){
      for (int i = 0; i < players[j].snakeLength; i++) {
        if (players[myNumber - 1].currentX == players[j].tailX[i] && players[myNumber - 1].currentY == players[j].tailY[i]) {
          byte payload = (byte)myNumber;

          Serial.print("Collision: ");
          Serial.print(players[j].tailX[i]);
          Serial.print(" ");
          Serial.println(players[j].tailY[i]);

          Serial.print("Direction: ");
          Serial.print(myNumber);
          Serial.print(" ");
          Serial.println(players[myNumber - 1].direction);

          client.publish("snake/death", &payload, 1);
          gameOver();
          return;
        }
      }
    }
    if (!gameStarted) {
      return;
    }
    updateLEDs();
    encoderHandled = false; // Reset encoder handling flag after each movement tick
  }

  if (encoderChanged && !encoderHandled) {
    encoderHandled = true;
    encoderChanged = false;
    int newDirection = encoderPosition % 4;
    if (newDirection < 0) newDirection += 4;
    if (newDirection != (players[myNumber-1].direction + 2) % 4) { // Prevent reversing direction
      // Publish the move to the MQTT topic
      byte payload[2] = { (byte)myNumber, (byte)newDirection };
      client.publish("snake/move", payload, 2);
    }
  }
}

void snakeCallback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, "snake/move") == 0){
    int playerNumber = payload[0];
    int newDirection = payload[1];
    players[playerNumber - 1].direction = newDirection;
  } 
  else if (strcmp(topic, "snake/join") == 0) {
    numPlayers++;
    if (myNumber == 1) {
      client.publish("snake/assign", (char *)&numPlayers, sizeof(numPlayers));
    }
    updateTitle = true;
  } 
  else if (strcmp(topic, "snake/assign") == 0) {
    Serial.print("Assigning number ");
    Serial.println(payload[0]);
    int playerNumber = payload[0];
    if (myNumber == 0) {
      myNumber = playerNumber;
      numPlayers = playerNumber;
      players[myNumber - 1].snakeColor = playerNumber;
      client.unsubscribe("snake/assign");
      updateTitle = true;
    }
  } 
  else if (strcmp(topic, "snake/start") == 0) {
    gameStarted = true;
  } 
  else if (strcmp(topic, "snake/color") == 0) {
    players[payload[0] - 1].snakeColor = payload[1];
  }
  else if (strcmp(topic, "snake/food") == 0) {
    foodX = payload[0];
    foodY = payload[1];
  }
  else if (strcmp(topic, "snake/death") == 0) {
    players[payload[0]].snakeLength = 0;
  }
}

void handleStartScreen() {
  while (!gameStarted) {
    if (updateTitle){
      updateTitle = false;
      char s[10]; 
      sprintf(s, "%d PLAYERS", numPlayers);
      const char* text = s;
      startScrollText(text, TEXT_SPEED);
    }
    updateScrollText(); // Update the scrolling text
    if (encoderChanged) {
      encoderChanged = false;
      int colorIndex = encoderPosition % 10;
      if (colorIndex < 0) colorIndex += 10;
      // Broadcast the color change
      byte payload[2] = { (byte)myNumber, (byte)colorIndex };
      client.publish("snake/color", payload, 2);
    }

    showStartScreen();

    buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == LOW && lastButtonState == HIGH) {
        spawnFood();
        updateLEDs();
        client.publish("snake/start", "start"); // Notify all players to start the game
    }
    lastButtonState = buttonState;
    delay(10);
    if (gameStarted){
      encoderPosition = players[myNumber - 1].direction;
    }
  }
}

void startSnakes() {
  myNumber = 0;
  resetGame();

  client.setCallback(snakeCallback);
  client.subscribe("snake/move");
  client.subscribe("snake/join");
  client.subscribe("snake/color");
  client.subscribe("snake/start");
  client.subscribe("snake/assign");
  client.subscribe("snake/food");
  client.subscribe("snake/death");

  // Publish a join request
  client.publish("snake/join", "join");

  // Wait for player number assignment with a timeout
  unsigned long startTime = millis();
  while (myNumber == 0 && millis() - startTime < 2000) {
    delay(10);
  }

  // If no response, assign player 1
  if (myNumber == 0) {
    myNumber = 1;
    numPlayers = 1;
    client.unsubscribe("snake/assign");
  }

  textColor = CRGB::White;
  char s[10]; 
  sprintf(s, "%d PLAYERS", numPlayers);
  const char* text = s;
  startScrollText(text, TEXT_SPEED);

  handleStartScreen();

  while (gameStarted) {
    gameLoop();
  }

  client.unsubscribe("snake/move");
  client.unsubscribe("snake/join");
  client.unsubscribe("snake/color");
  client.unsubscribe("snake/start");
  client.unsubscribe("snake/assign");
  client.unsubscribe("snake/food");
  client.unsubscribe("snake/death");
}