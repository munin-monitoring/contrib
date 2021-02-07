/*
 * Copyright (C) 2008 Helmut Grohne <helmut@subdivi.de> - All rights reserved.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU General Public License v.2 or v.3.
 */
#ifndef COMMON_H
#define COMMON_H

#define PROC_STAT "/proc/stat"

/** Write yes to stdout and return 0. The intended use is give an autoconf
 * response like "return writeyes();".
 * @returns a success state to be passed on as the return value from main */
int writeyes(void);

/** Answer an autoconf request by checking the readability of the given file.
 */
int autoconf_check_readable(const char *);

/** Obtain an integer value from the environment. In the absence of the
 * variable the given defaultvalue is returned.  */
int getenvint(const char *, int defaultvalue);

/** Print a name.warning line using the "name_warning" or "warning" environment
 * variables. */
void print_warning(const char *name);

/** Print a name.critical line using the "name_critical" or "critical"
 * environment variables. */
void print_critical(const char *name);

/** Print both name.warning and name.critical lines using environment
 * variables. */
void print_warncrit(const char *name);

/** Fail by printing the given message and a newline to stderr.
 * @returns a failure state to be passed on as the return value from main */
int fail(const char *message);

#define xisspace(x) isspace((int)(unsigned char) x)
#define xisdigit(x) isdigit((int)(unsigned char) x)

#endif
