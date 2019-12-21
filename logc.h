#ifndef __LOG_C_H__
#define __LOG_C_H__

#include <stdarg.h>
#include <sys/time.h>
#include <sys/uio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define logc_init_self(filename) log_init(filename, 0600)
#define logc_init_group(filename) log_init(filename, 0660)
#define logc_init_other(filename) log_init(filename, 0666)

#define logc_debug(logger, fmt, ...) logc_log(logger, LOG_DEBUG, fmt, ##__VA_ARGS__)
#define logc_info(logger, fmt, ...) logc_log(logger, LOG_INFO, fmt, ##__VA_ARGS__)
#define logc_warn(logger, fmt, ...) logc_log(logger, LOG_WORN, fmt, ##__VA_ARGS__)
#define logc_error(logger, fmt, ...) logc_log(logger, LOG_ERROR, fmt, ##__VA_ARGS__)
#define logc_vdebug(logger, fmt, ap) logc_vlog(logger, LOG_DEBUG, fmt, ap)
#define logc_vinfo(logger, fmt, ap) logc_vlog(logger, LOG_INFO, fmt, ap)
#define logc_vwarn(logger, fmt, ap) logc_vlog(logger, LOG_WORN, fmt, ap)
#define logc_verror(logger, fmt, ap) logc_vlog(logger, LOG_ERROR, fmt, ap)

#define logc_debug_binary(logger, data, data_size, fmt, ...) logc_binary(logger, LOG_DEBUG, data, data_size, fmt, ##__VA_ARGS__)
#define logc_info_binary(logger, data, data_size, fmt, ...) logc_binary(logger, LOG_INFO, data, data_size, fmt, ##__VA_ARGS__)
#define logc_warn_binary(logger, data, data_size, fmt, ...) logc_binary(logger, LOG_WORN, data, data_size, fmt, ##__VA_ARGS__)
#define logc_error_binary(logger, data, data_size, fmt, ...) logc_binary(logger, LOG_ERROR, data, data_size, fmt, ##__VA_ARGS__)
#define logc_vdebug_binary(logger, data, data_size, fmt, ap) logc_vbinary(logger, LOG_DEBUGdata, data_size, , fmt, ap)
#define logc_vinfo_binary(logger, data, data_size, fmt, ap) logc_vbinary(logger, LOG_INFO, data, data_size, fmt, ap)
#define logc_vwarn_binary(logger, data, data_size, fmt, ap) logc_vbinary(logger, LOG_WORN, data, data_size, fmt, ap)
#define logc_verror_binary(logger, data, data_size, fmt, ap) logc_vbinary(logger, LOG_ERROR, data, data_size, fmt, ap)


struct logc;
typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WORN,
    LOG_ERROR,
} LOGC_LOG_LEVEL;

typedef struct {
    char *buffer;
    size_t buffer_size;
} formatter_result_t;

typedef formatter_result_t (*date_formatter_t)(const struct timeval *now, char *buf, size_t buf_size);
typedef formatter_result_t (*time_formatter_t)(const struct timeval *now, char *buf, size_t buf_size);
typedef formatter_result_t (*level_formatter_t)(LOGC_LOG_LEVEL level, char *buf, size_t buf_size);
typedef formatter_result_t (*binary_formatter_t)(size_t max_print_size, const char *data, size_t data_size, char *buf, size_t buf_zie);

struct logc *logc_init(const char *filename, int mode);
void logc_free(struct logc *logger);

void logc_set_rotate(struct logc *logger, int rotate_count, ssize_t rotate_size);
void logc_set_date_formatter(struct logc *logger, date_formatter_t formatter);
void logc_set_time_formatter(struct logc *logger, time_formatter_t formatter);
void logc_set_level_formatter(struct logc *logger, level_formatter_t formatter);
/**
 * default max of binary data size is 1024
 * and default one byte buffer size is defined by macro BINARY_BYTES_PER_BYTE(16), you can change it by recompile logc library
 **/
void logc_set_binary_maxsize(struct logc *logger, size_t siz);
void lgoc_set_level(struct logc *logger, LOGC_LOG_LEVEL level);

void logc_log(struct logc *logger, LOGC_LOG_LEVEL level, const char *fmt, ...);
void logc_vlog(struct logc *logger, LOGC_LOG_LEVEL level, const char *fmt, va_list ap);

void logc_binary(struct logc *logger, LOGC_LOG_LEVEL level, const char *data, size_t data_size, const char *fmt, ...);
void logc_vbinary(struct logc *logger, LOGC_LOG_LEVEL level, const char *data, size_t data_size, const char *fmt, va_list ap);


#ifdef __cplusplus
} // extern C
#endif

#endif //__LOG_C_H__
