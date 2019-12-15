#include "logc.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/syslimits.h>
#include <sys/errno.h>

#define CHECK_POINTER(ptr) do {if (NULL == ptr) return;} while(0)

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif // BUFFER_SIZE

#define BUFFER_COUNT 4

#define VERIFY(var) do {if (!(var)) abort();} while(0)

#define DEFAULT_DATE_FORMAT
#define DEFAULT_TIME_FORMAT
#define DEFAULT_LEVEL_FORMAT

const char *_LEVEL_STRING [3] = {
    "DEBUG",
    "INFO",
    "ERROR",
};


struct logc {
    int fd;
    LOGC_LOG_LEVEL level;
    char filename[PATH_MAX];
    int mode;
    char *buffer[BUFFER_COUNT];
    ssize_t total;
    ssize_t rotate_size;
    uint32_t rotate_count;

    date_formatter_t date_formatter;
    time_formatter_t time_formatter;
    level_formatter_t level_formatter;
};

formatter_result_t _default_level_formatter(LOGC_LOG_LEVEL level, char *buf, size_t buf_size) {
    formatter_result_t result = {buf, 0};
    result.buffer_size = snprintf(buf, buf_size, "[%s] ", _LEVEL_STRING[level]);
    return result;
}

formatter_result_t _default_date_formatter(const struct timeval *now, char *buf, size_t buf_size) {
    formatter_result_t result = {buf, 0};
    result.buffer_size = strftime(buf, buf_size, "[%Y-%m-%d ", localtime(&now->tv_sec));
    return result;
}

formatter_result_t _default_time_formatter(const struct timeval *now, char *buf, size_t buf_size) {
    char tempbuf[6];
    int i = 0;

    formatter_result_t result = {buf, 0};
    result.buffer_size = strftime(buf, buf_size, "%H-%M-%S,", localtime(&now->tv_sec));
    snprintf(tempbuf, sizeof(tempbuf), "%03d] ", now->tv_usec/1000);
    while (result.buffer_size < buf_size - 1 && i < 5) {
        result.buffer[result.buffer_size] = tempbuf[i++];
        result.buffer_size += 1;
    }
    result.buffer[result.buffer_size] = 0;
    return result;
}

int _do_open(struct logc *logger) {
    if (-1 != logger->fd) {
        close(logger->fd);
    }

    logger->fd = open(logger->filename, O_RDWR|O_CREAT|O_APPEND, logger->mode);
    if (-1 == logger->fd) {
        return -1;
    }
    logger->total = lseek(logger->fd, 0, SEEK_END);
    return 0;
}

struct logc *logc_init(const char *filename, int mode) {
    struct logc *logger = NULL;
    int i;

    VERIFY(logger = malloc(sizeof(*logger)));
    strncpy(logger->filename, filename, sizeof(logger->filename));
    logger->mode = mode;

    if ( 0 != _do_open(logger) ) {
        free(logger);
        return NULL;
    }
    logger->rotate_count = 0;
    logger->rotate_size = 0;
    for ( i = 0;i < sizeof(logger->buffer)/sizeof(char *);i++) {
        VERIFY(logger->buffer[i] = malloc(BUFFER_SIZE));
    }
    logger->date_formatter = _default_date_formatter;
    logger->time_formatter = _default_time_formatter;
    logger->level_formatter = _default_level_formatter;
    return logger;
}

void logc_free(struct logc *logger) {
    int i;
    for ( i = 0;i < sizeof(logger->buffer)/sizeof(char *);i++) {
        free(logger->buffer[i]);
        logger->buffer[i] = NULL;
    }
    free(logger);
}

void logc_log(struct logc *logger, LOGC_LOG_LEVEL level, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    logc_vlog(logger, level, fmt, ap);
    va_end(ap);
}

void _do_rotate(struct logc *logger) {
    char origin_path[PATH_MAX], src_path[PATH_MAX], dst_path[PATH_MAX];
    int i, ret;
    if ( 0 != fcntl(logger->fd, F_GETPATH, origin_path) ) {
        // do nothing
        return;
    }
    for (i = logger->rotate_count;i > 1;i--) {
        snprintf(src_path, sizeof(src_path), "%s.%d", origin_path, i - 1);
        snprintf(dst_path, sizeof(dst_path), "%s.%d", origin_path, i);
        if (i == logger->rotate_count) {
            // remove last one
            ret = unlink(src_path);
        } else {
            ret = rename(src_path, dst_path);
        }
        if (ret != 0 && ENOENT != errno) {
            // stop rotate, wait for next time
            ret = errno;
            fprintf(stderr, "%d\n", ret);
            return;
        }
    }
    // do current file
    close(logger->fd);
    logger->fd = -1;
    // ignore return value
    rename(origin_path, src_path);

    _do_open(logger);
}

void logc_vlog(struct logc *logger, LOGC_LOG_LEVEL level, const char *fmt, va_list ap) {
    struct iovec vec[BUFFER_COUNT + 1];
    struct timeval tv;
    formatter_result_t result;
    ssize_t written;

    CHECK_POINTER(logger);

    if (level < logger->level) {
        return;
    }

    gettimeofday(&tv, NULL);

    result = logger->date_formatter(&tv, logger->buffer[0], BUFFER_SIZE);
    vec[0].iov_base = result.buffer;
    vec[0].iov_len = result.buffer_size;

    result = logger->time_formatter(&tv, logger->buffer[1], BUFFER_SIZE);
    vec[1].iov_base = result.buffer;
    vec[1].iov_len = result.buffer_size;

    result = logger->level_formatter(level, logger->buffer[2], BUFFER_SIZE);
    vec[2].iov_base = result.buffer;
    vec[2].iov_len = result.buffer_size;

    result = logger->level_formatter(level, logger->buffer[3], BUFFER_SIZE);
    vec[3].iov_base = logger->buffer[3];
    vec[3].iov_len = vsnprintf(logger->buffer[3], BUFFER_SIZE, fmt, ap);

    vec[4].iov_base = "\n";
    vec[4].iov_len = 1;

    written  = writev(logger->fd, vec, sizeof(vec)/sizeof(struct iovec));
    logger->total += written;

    if (logger->rotate_size && logger->rotate_count && logger->total > logger->rotate_size) {
        _do_rotate(logger);
    }
}

void logc_set_rotate(struct logc *logger, int rotate_count, ssize_t rotate_size) {
    logger->rotate_count = rotate_count;
    logger->rotate_size = rotate_size;
}

