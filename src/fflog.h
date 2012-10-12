#ifndef _LOG_H_
#define _LOG_H_
#include <stdio.h>
#ifdef DEBUG
//  macro to log fields in structs.
#define log_struct(st, field, format, typecast) \
  log_msg("    " #field " = " #format "\n", typecast st->field)

void log_fi (struct fuse_file_info *fi);
void log_stat(struct stat *si);
void log_statvfs(struct statvfs *sv);
void log_utime(struct utimbuf *buf);

void log_msg(const char *format, ...);
#else

#define log_fi(x)
#define log_stat(x)
#define log_statvfs(x)
#define log_utime(x)
#define log_msg(...)

#endif // DEBUG
#endif // _LOG_H_
