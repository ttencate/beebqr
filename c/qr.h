#ifndef QR_H
#define QR_H

#define DATA_BYTES 2953
#define DATA_CODEWORDS 2956
#define ERROR_CORRECTION_CODEWORDS 750
#define CODEWORDS (DATA_CODEWORDS + ERROR_CORRECTION_CODEWORDS)
#define MODULES_PER_SIDE 177
#define MODULES (MODULES_PER_SIDE * MODULES_PER_SIDE)

void init_log_tables();
void qr(const char *input, int count, unsigned char *output);

#endif
