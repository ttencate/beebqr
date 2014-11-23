#include "qr.h"

#include <png.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#define PROG "qr"

#define QUIET_MODULES_PER_EDGE 4
#define MODULES_PER_SIDE_INCLUDING_QUIET (MODULES_PER_SIDE + 2*QUIET_MODULES_PER_EDGE)
#define MODULES_INCLUDING_QUIET (MODULES_PER_SIDE_INCLUDING_QUIET * MODULES_PER_SIDE_INCLUDING_QUIET)
#define PIXELS_PER_MODULE 4
#define PIXELS_PER_SIDE (PIXELS_PER_MODULE * MODULES_PER_SIDE_INCLUDING_QUIET)
#define PIXELS (PIXELS_PER_SIDE * PIXELS_PER_SIDE)

int main() {
  // Read input
  char input[DATA_BYTES];
  FILE *in = stdin;
  int count = 0;
  while (count < DATA_BYTES && !ferror(in) && !feof(in)) {
    count += fread(input + count, 1, DATA_BYTES - count, in);
  }
  if (ferror(in)) {
    err(EXIT_FAILURE, "read error");
  }

  // Generate QR code
  unsigned char output[MODULES_INCLUDING_QUIET];
  qr(input, count, output);

  // Upscale QR code and add border
  unsigned char image[PIXELS];
  for (int y = 0; y < PIXELS_PER_SIDE; y++) {
    for (int x = 0; x < PIXELS_PER_SIDE; x++) {
      int i = y / PIXELS_PER_MODULE - QUIET_MODULES_PER_EDGE;
      int j = x / PIXELS_PER_MODULE - QUIET_MODULES_PER_EDGE;
      unsigned char pixel;
      if (i < 0 || i >= MODULES_PER_SIDE || j < 0 || j >= MODULES_PER_SIDE) {
        pixel = 0xff;
      } else {
        pixel = output[j * MODULES_PER_SIDE + i];
      }
      image[y * PIXELS_PER_SIDE + x] = pixel;
    }
  }
  
  // Write PNG file
  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    errx(EXIT_FAILURE, "error allocating png write struct");
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    errx(EXIT_FAILURE, "error allocating png info struct");
  }


  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    errx(EXIT_FAILURE, "libpng error");
  }

  png_set_IHDR(png_ptr, info_ptr,
      PIXELS_PER_SIDE, PIXELS_PER_SIDE,
      1, PNG_COLOR_TYPE_GRAY,
      PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  unsigned char *row_pointers[PIXELS_PER_SIDE];
  for (int y = 0; y < PIXELS_PER_SIDE; y++) {
    row_pointers[y] = image + y * PIXELS_PER_SIDE;
  }
  png_set_rows(png_ptr, info_ptr, row_pointers);

  png_init_io(png_ptr, stdout);
  png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_PACKING, NULL);

  return 0;
}
