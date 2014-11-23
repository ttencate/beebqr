#include "qr.h"

#include <stdbool.h>

#define DARK 0x00
#define LIGHT 0xff

const int alignment_patterns[] = {6, 30, 58, 86, 114, 142, 170};

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
  // Alignment patterns
  for (int m = 0; m < 7; m++) {
    if (i - alignment_patterns[m] >= -2 && i - alignment_patterns[m] <= 2) {
      for (int n = 0; n < 7; n++) {
        if (m == 0 && n == 0) continue;
        if (m == 0 && n == 6) continue;
        if (m == 6 && n == 0) continue;
        if (j - alignment_patterns[n] >= -2 && j - alignment_patterns[n] <= 2) {
          return true;
        }
      }
    }
  }
  // Timing patterns
  if (i == 6 || j == 6) {
    return true;
  }
  return false;
}

unsigned char finder_pattern_at(int i, int j) {
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

unsigned char alignment_pattern_at(int i, int j) {
  if (i == -2 || i == 2 || j == -2 || j == 2) {
    return DARK;
  }
  if (i == 0 && j == 0) {
    return DARK;
  }
  return LIGHT;
}

unsigned char function_pattern_at(int i, int j) {
  // Finder patterns
  if (i <= 7 && j <= 7) {
    return finder_pattern_at(i, j);
  }
  if (i >= MODULES_PER_SIDE - 8 && j <= 7) {
    return finder_pattern_at(i - MODULES_PER_SIDE + 7, j);
  }
  if (i <= 7 && j >= MODULES_PER_SIDE - 8) {
    return finder_pattern_at(i, j - MODULES_PER_SIDE + 7);
  }
  // Alignment patterns
  for (int m = 0; m < sizeof(alignment_patterns) / sizeof(int); m++) {
    if (i - alignment_patterns[m] >= -2 && i - alignment_patterns[m] <= 2) {
      for (int n = 0; n < sizeof(alignment_patterns) / sizeof(int); n++) {
        if (j - alignment_patterns[n] >= -2 && j - alignment_patterns[n] <= 2) {
          return alignment_pattern_at(i - alignment_patterns[m], j - alignment_patterns[n]);
        }
      }
    }
  }
  // Timing patterns
  return ((i+j) % 2) ? LIGHT : DARK;
}

void qr(const char *input, int count, unsigned char *output) {
  for (int i = 0; i < MODULES_PER_SIDE; i++) {
    for (int j = 0; j < MODULES_PER_SIDE; j++) {
      output[i * MODULES_PER_SIDE + j] =
        is_function_pattern(i, j) ? function_pattern_at(i, j) : LIGHT;
    }
  }
}
