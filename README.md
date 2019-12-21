# 1. BUILDING AND INSTALLATION

## Autoconf

```
$ ./autogen.sh
$ ./configure
$ make
$ sudo make install
```

# 2. Example

#include <logc.h>

int main() {
    struct logc *logger = NULL;

    logger = logc_init("/tmp/test.log", 0600);
    // Rotate is disable by default
    logc_set_rotate(logger, 10, 5000);
    lgoc_set_level(logger, LOG_INFO);

    // and you can set data format, time format and level format function

    logc_free(logger);
    return 0;
}

