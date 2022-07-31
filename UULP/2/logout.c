/**
 * Created by iccy on 22-7-17.
 *
 * logout - user logout
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <utmp.h>

extern int          utmp_open(char* filename);
extern struct utmp* utmp_next();
int                 utmp_seek(__off_t off, int base);
extern void         utmp_close();

/*
 * logout_tty(char* line)
 * marks a utmp record as logged out
 * does not blank username or remote host
 * return -1 on error, 0 on success
 */
int logout_tty(char* line) {
  int          fd = 0;
  struct utmp* utbufp;

  /* open file */
  if (-1 == (fd = utmp_open(UTMP_FILE))) {

    return -1;
  }

  /* search */
  while (NULL != (utbufp = utmp_next())) {
    if (!strncmp(utbufp->ut_line, line, sizeof(utbufp->ut_line))) {
      break;
    }
  }

  utbufp->ut_type = DEAD_PROCESS; /* set type */

  if (-1 == gettimeofday(utbufp->ut_tv, NULL)) {
    return -1;
  }

  if (-1 == utmp_seek(-1, SEEK_CUR)) {
    return -1;
  }

  if (-1 = write(fd, utbufp, sizeof(struct utmp))) {
    return -1;
  }

  /* close file */
  utmp_close();

  return 0;
}