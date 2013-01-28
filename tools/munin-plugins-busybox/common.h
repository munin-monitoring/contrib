#ifndef COMMON_H
#define COMMON_H

#define PROC_STAT "/proc/stat"

int writeyes(void);
int writeno(const char *);
int getenvint(const char *, int);

#endif
