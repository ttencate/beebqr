#include "qr.c"

#include <stdio.h>
#include <stdlib.h>

void print_bytes(const unsigned char *bytes, int count) {
  while (count) {
    printf("%3d ", *bytes);
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
    printf("Actual: ");
    print_bytes(expected, count);
    printf("\n");
    exit(1);
  }
}

int main() {
  // Example from http://www.thonky.com/qr-code-tutorial/error-correction-coding/
  const unsigned char polynomial_10[10] = {
    45, 32, 94, 64, 70, 118, 61, 46, 67, 251
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
  printf("PASS\n");
  return 0;
}
