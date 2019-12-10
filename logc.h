#ifndef __LOG_C_H__
#define __LOG_C_H__

struct logc {
};

struct logc *logc_init(const char *filename, int mode);
void logc_free(struct logc *logger);

#endif //__LOG_C_H__
