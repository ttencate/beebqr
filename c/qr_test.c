#include "qr.c"

#include <stdio.h>
#include <stdlib.h>

void print_bytes(const unsigned char *bytes, int count) {
  while (count) {
    printf("0x%02x ", *bytes);
    bytes++;
    count--;
  }
}

void assert_equal_bytes(const unsigned char *expected, const unsigned char *actual, int count) {
  if (memcmp(expected, actual, count)) {
    printf("FAIL\n");
    printf("Expected: ");
    print_bytes(expected, count);
    printf("\n");
    printf("Actual:   ");
    print_bytes(actual, count);
    printf("\n");
    exit(1);
  }
  printf("PASS\n");
}

void test_generate_data_codewords() {
  unsigned char data[10] = {
    0x00, 0x00, 0x00, 0x12, 0x00, 0x34, 0xff, 0x00, 0x00, 0x00
  };
  const unsigned char expected[10] = {
    0x40, 0x00, 0x41, 0x20, 0x03, 0x4f, 0xf0, 0xec, 0x11, 0xec
  };
  generate_data_codewords(4, data, 10);

  assert_equal_bytes(expected, data, 10);
}

void test_generate_error_correction() {
  // Example from http://www.thonky.com/qr-code-tutorial/error-correction-coding/
  const unsigned char polynomial_10[10] = {
    251, 67, 46, 61, 118, 70, 64, 94, 32, 45
  };
  const unsigned char data_codewords[16] = {
    32, 91, 11, 120, 209, 114, 220, 77, 67, 64, 236, 17, 236, 17, 236, 17
  };
  unsigned char error_codewords[10];
  generate_error_correction(data_codewords, 16, error_codewords, 10, polynomial_10);

  const unsigned char expected[] = {
    196, 35, 39, 119, 235, 215, 231, 226, 93, 23
  };
  assert_equal_bytes(expected, error_codewords, 10);
}

int main() {
  test_generate_data_codewords();
  test_generate_error_correction();
  return 0;
}
