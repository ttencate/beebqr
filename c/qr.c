#include "qr.h"

void qr(const char *input, int count, unsigned char *output) {
  for (int i = 0; i < MODULES_PER_SIDE; i++) {
    for (int j = 0; j < MODULES_PER_SIDE; j++) {
      output[i * MODULES_PER_SIDE + j] = 0xff * ((i+j) % 2);
    }
  }
  return;
}
