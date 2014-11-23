#include "qr.h"

#include <stdbool.h>

#define DARK 0x00
#define LIGHT 0xff

bool is_function_pattern(int i, int j) {
  // Finder patterns
  if (i < 8 && j < 8) {
    return true;
  }
  if (i >= MODULES_PER_SIDE - 8 && j < 8) {
    return true;
  }
  if (i < 8 && j >= MODULES_PER_SIDE - 8) {
    return true;
  }
  return false;
}

unsigned char function_pattern_at(int i, int j) {
  // Finder patterns
  if (i >= MODULES_PER_SIDE - 8) {
    i -= MODULES_PER_SIDE - 7;
  }
  if (j >= MODULES_PER_SIDE - 8) {
    j -= MODULES_PER_SIDE - 7;
  }
  if (i == -1 || i == 7 || j == -1 || j == 7) {
    return LIGHT;
  }
  if (i == 0 || i == 6 || j == 0 || j == 6) {
    return DARK;
  }
  if (i == 1 || i == 5 || j == 1 || j == 5) {
    return LIGHT;
  }
  return DARK;
}

void qr(const char *input, int count, unsigned char *output) {
  for (int i = 0; i < MODULES_PER_SIDE; i++) {
    for (int j = 0; j < MODULES_PER_SIDE; j++) {
      output[i * MODULES_PER_SIDE + j] =
        is_function_pattern(i, j) ? function_pattern_at(i, j) : LIGHT;
    }
  }
  return;
}
