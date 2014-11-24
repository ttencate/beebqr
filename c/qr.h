#ifndef QR_H
#define QR_H

#define DATA_BYTES 2953
#define MODULES_PER_SIDE 177
#define MODULES (MODULES_PER_SIDE * MODULES_PER_SIDE)

void qr(const char *input, int count, unsigned char *output);

#endif
