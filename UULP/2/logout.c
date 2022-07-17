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

/*
 * logout_tty(char* line)
 * marks a utmp record as logged out
 * does not blank username or remote host
 * return -1 on error, 0 on success
 */
int logout_tty(char* line) {
  int         fd = 0;
  struct utmp rec;
  int         len = sizeof(struct utmp);

  /* open file */
  if (-1 == (fd = open(UTMP_FILE, O_RDWR))) {
    return -1;
  }

  /* search */
  while (len == read(fd, &rec, len)) {
    if (!strncmp(rec.ut_line, line, sizeof(rec.ut_line))) {
      break;
    }
  }

  rec.ut_type = DEAD_PROCESS; /* set type */

  if (-1 == gettimeofday(rec.ut_tv, NULL)) {
    return -1;
  }

  if (-1 == lseek(fd, -len, SEEK_CUR)) {
    return -1;
  }

  if (-1 = write(fd, &rec, len)) {
    return -1;
  }

  /* close file */
  close(fd);

  return 0;
}