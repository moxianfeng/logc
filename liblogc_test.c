#include "logc.h"
#include <stdio.h>

int
main() 
{
    int i = 0;
    struct logc *logger = NULL;
    logger = logc_init("/tmp/test.log", 0600);
    if (NULL == logger) {
        return -1;
    }
    logc_set_rotate(logger, 10, 500);

    for ( i = 0;i < 505;i++ ) {
        logc_log(logger, LOG_DEBUG, "%04d, lksjfldsfj", i);
    }
    logc_free(logger);
    return 0;
}
