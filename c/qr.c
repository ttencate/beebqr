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

unsigned short encode_format_info(unsigned char data_mask_pattern) {
  // TODO handle non-000 data mask patterns (tabularize by hand for 8 possible values)
  unsigned short version_info = 0x23d6;
  unsigned short version_info_xor = 0x5412; // fixed xor mask to prevent all zeros
  return version_info ^ version_info_xor;
}

unsigned int encode_version_info() {
  return 0x28c69;
}

void generate_data_codewords(int count, unsigned char *data, int data_size) {
  // Actual data bytes have been placed in data[3] onwards.
  int i = 2;
  for (; i < count + 2; i++) {
    data[i] = (data[i] << 4) | (data[i+1] >> 4);
  }
  data[i] = data[i] << 4;
  i++; // Low 4 bits of last byte left at 0000: terminator.

  // Add padding to fill up data area
  const unsigned char PAD_CODEWORDS[] = {0xec, 0x11};
  for (int j = 0; i < data_size; i++, j = 1 - j) {
    data[i] = PAD_CODEWORDS[j];
  }

  // High 4 bits: 0100 = binary mode.
  // Low 4 bits: high 4 bits of data length.
  data[0] = 0x40 | (count >> 12);
  // Middle 8 bits of data length.
  data[1] = 0xff & (count >> 4);
  // High 4 bits: low 4 bits of data length.
  // Low 4 bits: first 4 bits of actual data (filled in later).
  data[2] |= 0xf0 & (count << 4);
}

static bool inited = false;
static unsigned char log[256];
static unsigned char antilog[256];

void init_log_tables() {
  if (inited) {
    return;
  }
  inited = true;

  int power = 1;
  int exponent = 0;
  do {
    log[power] = exponent;
    antilog[exponent] = power;

    power <<= 1;
    exponent++;
    if (power >= 256) {
      power ^= 285;
    }
  } while (exponent < 256);
}

// Generator polynomial: x^30 + alpha^a[0] * x^29 + ... + alpha^a[29] * 1
const unsigned char polynomial_30[30] = {
  41, 173, 145, 152, 216, 31, 179, 182, 50, 48,
  110, 86, 239, 96, 222, 125, 42, 173, 226, 193,
  224, 130, 156, 37, 251, 216, 238, 40, 192, 180
};

void generate_error_correction(const unsigned char *data_codewords, int num_data_codewords, unsigned char *error_codewords, int num_error_codewords, const unsigned char *polynomial) {
  init_log_tables();

  memset(error_codewords, 0, num_error_codewords);
  for (int i = 0; i < num_data_codewords + num_error_codewords; i++) {
    // Remember the head so we can multiply the generator by it (minus its head).
    unsigned char head = *error_codewords;
    // Shift out the head to the left, shift in the next codeword to the right.
    for (int j = 0; j < num_error_codewords-1; j++) {
      error_codewords[j] = error_codewords[j+1];
    }
    if (i < num_data_codewords) {
      error_codewords[num_error_codewords-1] = *data_codewords;
      data_codewords++;
    } else {
      error_codewords[num_error_codewords-1] = 0;
    }
    // Add the generator, multiplied by the head coefficient that has been
    // shifted out, annihiliating the implicit head term.
    if (head) {
      for (int j = 0; j < num_error_codewords; j++) {
        error_codewords[j] ^= antilog[(log[head] + polynomial[j]) % 255];
      }
    }
  }
}

void generate_error_codewords(unsigned char *data, unsigned char *error) {
  // 19 blocks of 118, then 6 blocks of 119. Each block gets 30 error
  // correction codewords.
  for (int i = 0; i < 19; i++) {
    generate_error_correction(data, 118, error, 30, polynomial_30);
    data += 118;
    error += 30;
  }
  for (int i = 0; i < 6; i++) {
    generate_error_correction(data, 119, error, 30, polynomial_30);
    data += 119;
    error += 30;
  }
}

void write_byte(unsigned char *output, int *pi, int *pj, int *pdirection, unsigned char byte) {
  int i = *pi;
  int j = *pj;
  int direction = *pdirection;
  for (unsigned char mask = 0x80; mask; mask >>= 1) {
    // Advance writing position
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

    bool bit = (byte & mask);
    if ((i + j) % 2 == 0) {
      bit = !bit;
    }
    output[i * MODULES_PER_SIDE + j] = bit ? DARK : LIGHT;
  }
  *pi = i;
  *pj = j;
  *pdirection = direction;
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
  unsigned short format_info = encode_format_info(0x0);
  output[0 * MODULES_PER_SIDE + 8] = format_info & 0x0001 ? DARK : LIGHT;
  output[1 * MODULES_PER_SIDE + 8] = format_info & 0x0002 ? DARK : LIGHT;
  output[2 * MODULES_PER_SIDE + 8] = format_info & 0x0004 ? DARK : LIGHT;
  output[3 * MODULES_PER_SIDE + 8] = format_info & 0x0008 ? DARK : LIGHT;
  output[4 * MODULES_PER_SIDE + 8] = format_info & 0x0010 ? DARK : LIGHT;
  output[5 * MODULES_PER_SIDE + 8] = format_info & 0x0020 ? DARK : LIGHT;
  output[7 * MODULES_PER_SIDE + 8] = format_info & 0x0040 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + 8] = format_info & 0x0080 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + 7] = format_info & 0x0100 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + 5] = format_info & 0x0200 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + 4] = format_info & 0x0400 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + 3] = format_info & 0x0800 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + 2] = format_info & 0x1000 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + 1] = format_info & 0x2000 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + 0] = format_info & 0x4000 ? DARK : LIGHT;

  output[8 * MODULES_PER_SIDE + MODULES_PER_SIDE - 1] = format_info & 0x0001 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + MODULES_PER_SIDE - 2] = format_info & 0x0002 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + MODULES_PER_SIDE - 3] = format_info & 0x0004 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + MODULES_PER_SIDE - 4] = format_info & 0x0008 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + MODULES_PER_SIDE - 5] = format_info & 0x0010 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + MODULES_PER_SIDE - 6] = format_info & 0x0020 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + MODULES_PER_SIDE - 7] = format_info & 0x0040 ? DARK : LIGHT;
  output[8 * MODULES_PER_SIDE + MODULES_PER_SIDE - 8] = format_info & 0x0080 ? DARK : LIGHT;
  output[(MODULES_PER_SIDE - 7) * MODULES_PER_SIDE + 8] = format_info & 0x0100 ? DARK : LIGHT;
  output[(MODULES_PER_SIDE - 6) * MODULES_PER_SIDE + 8] = format_info & 0x0200 ? DARK : LIGHT;
  output[(MODULES_PER_SIDE - 5) * MODULES_PER_SIDE + 8] = format_info & 0x0400 ? DARK : LIGHT;
  output[(MODULES_PER_SIDE - 4) * MODULES_PER_SIDE + 8] = format_info & 0x0800 ? DARK : LIGHT;
  output[(MODULES_PER_SIDE - 3) * MODULES_PER_SIDE + 8] = format_info & 0x1000 ? DARK : LIGHT;
  output[(MODULES_PER_SIDE - 2) * MODULES_PER_SIDE + 8] = format_info & 0x2000 ? DARK : LIGHT;
  output[(MODULES_PER_SIDE - 1) * MODULES_PER_SIDE + 8] = format_info & 0x4000 ? DARK : LIGHT;
  output[(MODULES_PER_SIDE - 8) * MODULES_PER_SIDE + 8] = DARK;

  // Version information
  unsigned int version_info = encode_version_info();
  for (int bit = 0; bit < 18; bit++) {
    unsigned char module = version_info & (1 << bit) ? DARK : LIGHT;
    output[(MODULES_PER_SIDE - 11 + bit%3) * MODULES_PER_SIDE + (bit/3)] = module;
    output[(bit/3) * MODULES_PER_SIDE + (MODULES_PER_SIDE - 11 + bit%3)] = module;
  }

  // Copy data and add header
  unsigned char data[DATA_CODEWORDS];
  memset(data, 0, DATA_CODEWORDS);
  memcpy(data + 3, input, count); // On the Beeb, we'd read from disk here.
  generate_data_codewords(count, data, DATA_CODEWORDS);

  // Generate error correction codewords
  unsigned char error[ERROR_CODEWORDS];
  memset(error, 0, ERROR_CODEWORDS);
  generate_error_codewords(data, error);

  // Write codewords into the matrix
  int i = MODULES_PER_SIDE - 1, j = MODULES_PER_SIDE;
  int direction = 1;
  // 19 blocks of 118, then 6 blocks of 119
  for (int byte_in_block = 0; byte_in_block < 119; byte_in_block++) {
    if (byte_in_block < 118) {
      for (int block = 0; block < 19; block++) {
        write_byte(output, &i, &j, &direction, data[118 * block + byte_in_block]);
      }
    }
    for (int block = 0; block < 6; block++) {
      write_byte(output, &i, &j, &direction, data[118 * 19 + 119 * block + byte_in_block]);
    }
  }
  for (int byte_in_block = 0; byte_in_block < 30; byte_in_block++) {
    for (int block = 0; block < 25; block++) {
      write_byte(output, &i, &j, &direction, error[30 * block + byte_in_block]);
    }
  }
}
