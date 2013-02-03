#ifndef COMMON_H
#define COMMON_H

#define PROC_STAT "/proc/stat"

int writeyes(void);
int writeno(const char *);
int getenvint(const char *, int);
const char *getenv_composed(const char *, const char *);
void print_warning(const char *);
void print_critical(const char *);
void print_warncrit(const char *);

#endif
