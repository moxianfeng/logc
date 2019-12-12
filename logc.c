#include "logc.h"
#include <stdio.h>

struct logc {
};

struct logc *logc_init(const char *filename, int mode) {
    fprintf(stdout, "just test\n");
    return NULL;
}

void logc_free(struct logc *logger) {
}
