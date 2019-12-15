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

struct logc;
typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_ERROR,
} LOGC_LOG_LEVEL;

typedef struct {
    char *buffer;
    size_t buffer_size;
} formatter_result_t;

typedef formatter_result_t (*date_formatter_t)(const struct timeval *now, char *buf, size_t buf_size);
typedef formatter_result_t (*time_formatter_t)(const struct timeval *now, char *buf, size_t buf_size);
typedef formatter_result_t (*level_formatter_t)(LOGC_LOG_LEVEL level, char *buf, size_t buf_size);

struct logc *logc_init(const char *filename, int mode);
void logc_free(struct logc *logger);

void logc_log(struct logc *logger, LOGC_LOG_LEVEL level, const char *fmt, ...);
void logc_vlog(struct logc *logger, LOGC_LOG_LEVEL level, const char *fmt, va_list ap);

void logc_set_rotate(struct logc *logger, int rotate_count, ssize_t rotate_size);

#ifdef __cplusplus
} // extern C
#endif

#endif //__LOG_C_H__
