#ifndef __LOG_C_H__
#define __LOG_C_H__

#define logc_init_self(filename) log_init(filename, 0600)
#define logc_init_group(filename) log_init(filename, 0660)
#define logc_init_other(filename) log_init(filename, 0666)

struct logc;

struct logc *logc_init(const char *filename, int mode);
void logc_free(struct logc *logger);

#endif //__LOG_C_H__
