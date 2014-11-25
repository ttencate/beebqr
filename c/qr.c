#include "qr.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define DARK 0x00
#define LIGHT 0xff
#define DEBUG_GREY 0x80

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

bool is_format_info(int i, int j) {
  if (i == 8 && (j < 6 || j == 7 || j == 8 || j >= MODULES_PER_SIDE - 8)) {
    return true;
  }
  if (j == 8 && (i < 6 || i == 7 || i == 8 || i >= MODULES_PER_SIDE - 8)) {
    return true;
  }
  return false;
}

bool is_version_info(int i, int j) {
  if (i >= MODULES_PER_SIDE - 11 && i < MODULES_PER_SIDE - 8 && j < 6) {
    return true;
  }
  if (j >= MODULES_PER_SIDE - 11 && j < MODULES_PER_SIDE - 8 && i < 6) {
    return true;
  }
  return false;
}

bool is_data_pattern(int i, int j) {
  return !is_function_pattern(i, j) && !is_format_info(i, j) && !is_version_info(i, j);
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

unsigned short encode_version_info(unsigned char data_mask_pattern) {
  // TODO handle non-000 data mask patterns (tabularize by hand for 8 possible values)
  unsigned short version_info = 0x23d6;
  unsigned short version_info_xor = 0x5412; // fixed xor mask to prevent all zeros
  return version_info ^ version_info_xor;
}

void encode_data(const char *input, int count, unsigned char *data) {
  // Start with zeros to ensure ORing works
  memset(data, 0, CODEWORDS);

  // Copy data nibbles
  int i = 0;
  // High 4 bits: 0100 = binary mode.
  // Low 4 bits: high 4 bits of data length.
  data[i++] = 0x40 | (count >> 12);
  // Middle 8 bits of data length.
  data[i++] = 0xff & (count >> 4);
  // High 4 bits: low 4 bits of data length.
  // Low 4 bits: first 4 bits of actual data (filled in later).
  data[i] = 0xf0 & (count << 4);
  for (; i < count;) {
    unsigned char byte = input[i];
    data[i++] |= (byte >> 4);
    data[i] |= 0xf0 & (byte << 4);
  }
  i++; // Low 4 bits of last byte left at 0000: terminator.

  // Add padding to fill up data area
  const unsigned char PAD_CODEWORDS[] = {0xec, 0x11};
  for (int j = 0; i < DATA_CODEWORDS; i++, j = 1 - j) {
    data[i] = PAD_CODEWORDS[j];
  }

  // TODO add error correction codewords
}

void qr(const char *input, int count, unsigned char *output) {
  // Function patterns
  for (int i = 0; i < MODULES_PER_SIDE; i++) {
    for (int j = 0; j < MODULES_PER_SIDE; j++) {
      output[i * MODULES_PER_SIDE + j] =
        is_function_pattern(i, j) ? function_pattern_at(i, j) : LIGHT;
    }
  }

  // Format information
  unsigned short version_info = encode_version_info(0x0);
  output[0 * MODULES_PER_SIDE + 8] = version_info & 0x0001 ? DARK : LIGHT;
  output[1 * MODULES_PER_SIDE + 8] = version_info & 0x0002 ? DARK : LIGHT;
  output[2 * MODULES_PER_SIDE + 8] = version_info & 0x0004 ? DARK : LIGHT;
  output[3 * MODULES_PER_SIDE + 8] = version_info & 0x0008 ? DARK : LIGHT;
  output[4 * MODULES_PER_SIDE + 8] = version_info & 0x0010 ? DARK : LIGHT;
  output[5 * MODULES_PER_SIDE + 8] = version_info & 0x0020 ? DARK : LIGHT;
  output[7 * MODULES_PER_SIDE + 8] = version_info & 0x0040 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + 8] = version_info & 0x0080 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + 7] = version_info & 0x0100 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + 5] = version_info & 0x0200 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + 4] = version_info & 0x0400 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + 3] = version_info & 0x0800 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + 2] = version_info & 0x1000 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + 1] = version_info & 0x2000 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + 0] = version_info & 0x4000 ? DARK : LIGHT;

  output[8 * MODULES_PER_SIDE + MODULES_PER_SIDE - 1] = version_info & 0x0001 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + MODULES_PER_SIDE - 2] = version_info & 0x0002 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + MODULES_PER_SIDE - 3] = version_info & 0x0004 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + MODULES_PER_SIDE - 4] = version_info & 0x0008 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + MODULES_PER_SIDE - 5] = version_info & 0x0010 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + MODULES_PER_SIDE - 6] = version_info & 0x0020 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + MODULES_PER_SIDE - 7] = version_info & 0x0040 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + MODULES_PER_SIDE - 8] = version_info & 0x0080 ? DARK : LIGHT;
  output[(MODULES_PER_SIDE - 7) * MODULES_PER_SIDE + 8] = version_info & 0x0100 ? DARK : LIGHT;
  output[(MODULES_PER_SIDE - 6) * MODULES_PER_SIDE + 8] = version_info & 0x0200 ? DARK : LIGHT;
  output[(MODULES_PER_SIDE - 5) * MODULES_PER_SIDE + 8] = version_info & 0x0400 ? DARK : LIGHT;
  output[(MODULES_PER_SIDE - 4) * MODULES_PER_SIDE + 8] = version_info & 0x0800 ? DARK : LIGHT;
  output[(MODULES_PER_SIDE - 3) * MODULES_PER_SIDE + 8] = version_info & 0x1000 ? DARK : LIGHT;
  output[(MODULES_PER_SIDE - 2) * MODULES_PER_SIDE + 8] = version_info & 0x2000 ? DARK : LIGHT;
  output[(MODULES_PER_SIDE - 1) * MODULES_PER_SIDE + 8] = version_info & 0x4000 ? DARK : LIGHT;
  output[(MODULES_PER_SIDE - 8) * MODULES_PER_SIDE + 8] = DARK;

  // Data and error correction
  unsigned char data[CODEWORDS];
  encode_data(input, count, data);
  int i = MODULES_PER_SIDE - 1, j = MODULES_PER_SIDE - 1;
  int direction = -1;
  fprintf(stderr, "%d\n", data[0]);
  for (int bit = 0; bit < 8 * CODEWORDS; bit++) {
    unsigned char module = data[bit / 8] & (1 << (7 - bit % 8)) ? DARK : LIGHT;
    // unsigned char module = (unsigned char) ((bit * 0x20) % 0x100);
    output[i * MODULES_PER_SIDE + j] = module;
    if (bit < 8 * CODEWORDS + 1) {
      do {
        if (j % 2 == 0) {
          if (j < 6) j++; else j--;
        } else {
          if (j < 6) j--; else j++;
          i += direction;
          if (i < 0) {
            direction = -direction;
            i++;
            j -= 2;
            if (j == 6) j--; // Skip vertical timing pattern
          } else if (i >= MODULES_PER_SIDE) {
            direction = -direction;
            i--;
            j -= 2;
            if (j == 6) j--; // Skip vertical timing pattern
          }
        }
      }
      while (!is_data_pattern(i, j));
    }
  }
}
