#include "logc.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include <sys/syslimits.h>
#include <sys/errno.h>

#define CHECK_POINTER(ptr) do {if (NULL == ptr) return;} while(0)

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif // BUFFER_SIZE

#define BUFFER_COUNT 4

#define VERIFY(var) do {if (!(var)) abort();} while(0)

#ifndef DEFAULT_BINARY_MAXSIZE
#define DEFAULT_BINARY_MAXSIZE 1024
#endif // DEFAULT_BINARY_MAXSIZE

#ifndef BINARY_BUFFER_BYTES_PER_BYTE
#define BINARY_BUFFER_BYTES_PER_BYTE 16
#endif // BINARY_BUFFER_BYTES_PER_BYTE

#ifndef BINARY_COLS
#define BINARY_COLS 32
#endif // BINARY_COLS


const char *_LEVEL_STRING [4] = {
    "DEBUG",
    "INFO",
    "WORN",
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
    size_t binary_maxsize;

    char *binary_buffer;
    size_t binary_buffer_size;

    date_formatter_t date_formatter;
    time_formatter_t time_formatter;
    level_formatter_t level_formatter;
    binary_formatter_t binary_formatter;
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

int _default_binary_format_line(size_t lineno, const char *data, size_t data_size, char *buffer, size_t buffer_size) {
    int ret = 0;
    size_t i;
    int offset = 0;

    // `0000 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16  ................\n`
    size_t line_size = 4 + 3 * BINARY_COLS + 2 + BINARY_COLS + 1;

    // is the buffer enough for one line
    if ( buffer_size - offset < line_size ) {
        return 0;
    }

    // `0000 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16  ................\n`
    ret = snprintf(buffer + offset, buffer_size - offset, "%04lu", lineno);
    offset += ret;

    for ( i = 0;i < BINARY_COLS;i++ ) {
        unsigned char c = *(unsigned char *)(data + i);
        ret = snprintf(buffer + offset, buffer_size - offset, " %02x", c);
        offset += ret;

        if ( c >= 32 && c <= 126 ) {
            buffer[4 + 3 * BINARY_COLS + 2 + i] = c;
        } else {
            buffer[4 + 3 * BINARY_COLS + 2 + i] = '.';
        }
    }
    // `0000 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16  ................\n`
    //                                                      ^^
    buffer[4 + 3 * BINARY_COLS] = ' ';
    buffer[4 + 3 * BINARY_COLS + 1] = ' ';
    buffer[line_size - 1] = '\n';
    return line_size;
}

formatter_result_t _default_binary_formmater(size_t max_print_size, const char *data, size_t data_size, char *buffer, size_t buffer_size) {
    size_t i, buffer_offset = 0;
    int ret, j;
    formatter_result_t result = {buffer, 0};

    // `     01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16\n`
    size_t head_size = 4 + 3 * BINARY_COLS + 1;

    if ( buffer_size - buffer_offset < head_size ) {
        goto final_return;
    }
    // print column head
    // 0000 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16
    ret = snprintf(buffer + buffer_offset, buffer_size - buffer_offset, "%4s", " ");
    buffer_offset += ret;

    for ( i = 0;i < BINARY_COLS;i++ ) {
        ret = snprintf(buffer + buffer_offset, buffer_size - buffer_offset, " %02lu", i);
        buffer_offset += ret;
    }
    *(buffer + buffer_offset) = '\n';
    buffer_offset += 1;

    for ( i = 0;i < data_size && i < max_print_size;i+=BINARY_COLS ) {
        ret = _default_binary_format_line(i/BINARY_COLS, data + i, data_size - i, buffer + buffer_offset, buffer_size - buffer_offset);
        if ( ret == 0 ) {
            // buffer not enough
            break;
        }
        buffer_offset += ret;
    }

final_return:
    result.buffer_size = buffer_offset;
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
    logger->binary_maxsize = DEFAULT_BINARY_MAXSIZE;
    for ( i = 0;i < sizeof(logger->buffer)/sizeof(char *);i++) {
        VERIFY(logger->buffer[i] = malloc(BUFFER_SIZE));
    }
    logger->date_formatter = _default_date_formatter;
    logger->time_formatter = _default_time_formatter;
    logger->level_formatter = _default_level_formatter;
    logger->binary_formatter = _default_binary_formmater;

    logger->binary_buffer_size = logger->binary_maxsize * BINARY_BUFFER_BYTES_PER_BYTE;
    logger->binary_buffer = (char *)malloc(logger->binary_buffer_size);

    return logger;
}

void logc_free(struct logc *logger) {
    int i;
    for ( i = 0;i < sizeof(logger->buffer)/sizeof(char *);i++) {
        free(logger->buffer[i]);
        logger->buffer[i] = NULL;
    }
    if ( logger->binary_buffer ) {
        free(logger->binary_buffer);
        logger->binary_buffer = NULL;
    }
    free(logger);
}

void logc_set_date_formatter(struct logc *logger, date_formatter_t formatter) {
    logger->date_formatter = formatter;
}

void logc_set_time_formatter(struct logc *logger, time_formatter_t formatter) {
    logger->time_formatter = formatter;
}

void logc_set_level_formatter(struct logc *logger, level_formatter_t formatter) {
    logger->level_formatter = formatter;
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
    logc_vbinary(logger, level, NULL, 0, fmt, ap);
}

void logc_set_rotate(struct logc *logger, int rotate_count, ssize_t rotate_size) {
    logger->rotate_count = rotate_count;
    logger->rotate_size = rotate_size;
}

void logc_set_binary_maxsize(struct logc *logger, size_t siz) {
    logger->binary_maxsize = siz;

    logger->binary_buffer_size = logger->binary_maxsize * BINARY_BUFFER_BYTES_PER_BYTE;
    if (logger->binary_buffer ) {
        free(logger->binary_buffer);
        logger->binary_buffer = NULL;
    }
    logger->binary_buffer = malloc(logger->binary_buffer_size);
}

void logc_binary(struct logc *logger, LOGC_LOG_LEVEL level, const char *data, size_t data_size, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    logc_vbinary(logger, level, data, data_size, fmt, ap);
    va_end(ap);
}

void logc_vbinary(struct logc *logger, LOGC_LOG_LEVEL level, const char *data, size_t data_size, const char *fmt, va_list ap) {
    struct iovec *vec;
    int vec_count = 0;
    struct timeval tv;
    formatter_result_t result;
    ssize_t written;

    formatter_result_t binary_result = {NULL, 0};

    if ( data && data_size > 0 ) {
        // log binary data
        vec_count = BUFFER_COUNT + 2;
    } else {
        vec_count = BUFFER_COUNT + 1;
    }

    vec  = (struct iovec *)alloca(sizeof(struct iovec) * vec_count);

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

    if ( data && data_size > 0 ) {
        // log binary data
        binary_result = logger->binary_formatter(logger->binary_maxsize, data, data_size, logger->binary_buffer, logger->binary_buffer_size);
        vec[5].iov_base = binary_result.buffer;
        vec[5].iov_len = binary_result.buffer_size;
    }

    written  = writev(logger->fd, vec, vec_count);
    logger->total += written;

    if (logger->rotate_size && logger->rotate_count && logger->total > logger->rotate_size) {
        _do_rotate(logger);
    }
}

