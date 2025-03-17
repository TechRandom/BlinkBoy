#include "text.h"
#include "app.h"

const char* scrollText;
int scrollDelay;
unsigned long lastScrollTime;
int textLength;
int totalColumns;
int scrollOffset;

CRGB textColor = CRGB::Red;

void startScrollText(const char* text, int delayTime) {
  scrollText = text;
  scrollDelay = delayTime;
  lastScrollTime = millis();
  textLength = strlen(text);
  totalColumns = 0;
  for (int i = 0; i < textLength; i++) {
    totalColumns += getCharWidth(text[i]) + 1; // Add character width + 1 for space
  }
  totalColumns += MATRIX_WIDTH; // Add width of the matrix
  scrollOffset = 0;
}

void updateScrollText() {
  if (millis() - lastScrollTime >= scrollDelay) {
    lastScrollTime = millis();
    FastLED.clear();
    int currentOffset = scrollOffset;
    for (int i = 0; i < textLength; i++) {
      int charWidth = getCharWidth(scrollText[textLength - 1 - i]);
      displayChar(scrollText[textLength - 1 - i], currentOffset - charWidth);
      currentOffset -= (charWidth + 1); // Adjust for character width + 1 for space
    }
    scrollOffset--;
    if (scrollOffset < 0) {
      scrollOffset = totalColumns;
    }
  }
}

int getCharWidth(char c) {
  switch (c) {
    case 'A': return A[0];
    case 'B': return B[0];
    case 'C': return C[0];
    case 'D': return D[0];
    case 'E': return E[0];
    case 'F': return F[0];
    case 'G': return G[0];
    case 'H': return H[0];
    case 'I': return I[0];
    case 'J': return J[0];
    case 'K': return K[0];
    case 'L': return L[0];
    case 'M': return M[0];
    case 'N': return N[0];
    case 'O': return O[0];
    case 'P': return P[0];
    case 'Q': return Q[0];
    case 'R': return R[0];
    case 'S': return S[0];
    case 'T': return T[0];
    case 'U': return U[0];
    case 'V': return V[0];
    case 'W': return W[0];
    case 'X': return X[0];
    case 'Y': return Y[0];
    case 'Z': return Z[0];
    case '0': return ZERO[0];
    case '1': return ONE[0];
    case '2': return TWO[0];
    case '3': return THREE[0];
    case '4': return FOUR[0];
    case '5': return FIVE[0];
    case '6': return SIX[0];
    case '7': return SEVEN[0];
    case '8': return EIGHT[0];
    case '9': return NINE[0];
    case ' ': return SPACE[0];
    default: return 0;
  }
}

int displayChar(char c, int offset) {
  const byte* charMap;
  switch (c) {
    case 'A': charMap = A; break;
    case 'B': charMap = B; break;
    case 'C': charMap = C; break;
    case 'D': charMap = D; break;
    case 'E': charMap = E; break;
    case 'F': charMap = F; break;
    case 'G': charMap = G; break;
    case 'H': charMap = H; break;
    case 'I': charMap = I; break;
    case 'J': charMap = J; break;
    case 'K': charMap = K; break;
    case 'L': charMap = L; break;
    case 'M': charMap = M; break;
    case 'N': charMap = N; break;
    case 'O': charMap = O; break;
    case 'P': charMap = P; break;
    case 'Q': charMap = Q; break;
    case 'R': charMap = R; break;
    case 'S': charMap = S; break;
    case 'T': charMap = T; break;
    case 'U': charMap = U; break;
    case 'V': charMap = V; break;
    case 'W': charMap = W; break;
    case 'X': charMap = X; break;
    case 'Y': charMap = Y; break;
    case 'Z': charMap = Z; break;
    case '0': charMap = ZERO; break;
    case '1': charMap = ONE; break;
    case '2': charMap = TWO; break;
    case '3': charMap = THREE; break;
    case '4': charMap = FOUR; break;
    case '5': charMap = FIVE; break;
    case '6': charMap = SIX; break;
    case '7': charMap = SEVEN; break;
    case '8': charMap = EIGHT; break;
    case '9': charMap = NINE; break;
    case ' ': charMap = SPACE; break;
    default: return 0;
  }

  for (int col = 0; col < charMap[0]; col++) { // Adjust column count based on character width
    int displayCol = col + offset;
    if (displayCol >= 0 && displayCol < MATRIX_WIDTH) {
      for (int row = 0; row < MATRIX_HEIGHT; row++) {
        int pixelIndex = displayCol * MATRIX_HEIGHT + (MATRIX_HEIGHT - 1 - row);
        if (charMap[col + 1] & (1 << row)) {
          leds[pixelIndex] = textColor; 
        } 
        else {
          leds[pixelIndex] = CRGB::Black; 
        }
      }
    }
  }
  return 5 - charMap[0];
}