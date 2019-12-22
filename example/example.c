#include "logc.h"

int main() {
    struct logc *logger = NULL;
    int i;
    const char binarydata[] = "datal\0ksdjfls\tdjflsdjflsdfjlsdfjdslkfjdslkfjdslkfjdslkfjdlsfjlsdfjdslfjldsfjdlsfjldsfj";

    logger = logc_init("/tmp/test.log", 0600);
    // Rotate is disable by default
    logc_set_rotate(logger, 10, 500);
    lgoc_set_level(logger, LOG_INFO);

    // test rotate
    for ( i = 0;i < 20;i++ ) {
        logc_info_binary(logger, binarydata, sizeof(binarydata), "[%s:%d] %s, %d times", __FILE__, __LINE__, "log binary", i);
    }

    // and you can set data format, time format and level format function
    logc_debug(logger, "[%s:%d] %s", __FILE__, __LINE__, "log nothing, because level not enough");
    logc_info(logger, "[%s:%d] %s", __FILE__, __LINE__, "log it");
    logc_info_binary(logger, binarydata, sizeof(binarydata), "[%s:%d] %s", __FILE__, __LINE__, "log binary");


    logc_free(logger);
    return 0;
}

