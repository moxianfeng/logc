#include "logc.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int
main() 
{
    int i = 0;
    struct logc *logger = NULL;
    struct timeval tv;
    long data_size;
    char *data;

    gettimeofday(&tv, NULL);

    srand(tv.tv_usec);

    // [16, (2047 + 16)]
    data_size = 16 + random() % 2048;
    data = alloca(data_size);
    for ( i = 0;i < data_size;i++ ) {
        data[i] = (char)(random() % 128);
    }

    logger = logc_init("/tmp/test.log", 0600);
    if (NULL == logger) {
        return -1;
    }
    logc_set_rotate(logger, 10, 5000);

    for ( i = 0;i < 100;i++ ) {
        logc_error(logger, "%04d, lksjfldsfj", i);
        logc_log_binary(logger, LOG_DEBUG, data, data_size, "%04d, random binary data:", i);
    }
    logc_free(logger);
    return 0;
}
