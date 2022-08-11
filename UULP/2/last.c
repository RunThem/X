/**
 * Created by iccy on 22-8-2.
 */

#define _GNU_SOURCE

#define __WORDSIZE_TIME64_COMPAT32 0

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <utmp.h>

/*
 * FIX: why __WORDSIZE_TIME64_COMPAT32 true ???
 */
#if 0
#if __WORDSIZE_TIME64_COMPAT32
  int32_t ut_session;		/* Session ID, used for windowing.  */
  struct
  {
    int32_t tv_sec;		/* Seconds.  */
    int32_t tv_usec;		/* Microseconds.  */
  } ut_tv;			/* Time entry was made.  */
#else
  long int ut_session;		/* Session ID, used for windowing.  */
  struct timeval ut_tv;		/* Time entry was made.  */
#endif
#endif

int main(int argc, const char* argv[]) {
  int         count    = 0;
  int         fd       = -1;
  struct utmp ut_buf   = {0};
  char        buf[100] = {0};
  time_t      tm       = 0;

  if (argc == 2) {
    count = atoi(argv[1]);
  }

  fd = open(UTMP_FILE, O_RDONLY);
  if (-1 == fd) {
    exit(1);
  }

  if (count) {
    lseek(fd, -count * sizeof(struct utmp), SEEK_END);
  }

  while (0 != read(fd, &ut_buf, sizeof(struct utmp))) {
    tm = ut_buf.ut_tv.tv_sec;
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&tm));
    printf("%s\t\t%s\t%s\t\t%s\n", ut_buf.ut_user, ut_buf.ut_line,
           ut_buf.ut_host, buf);
  }

  return 0;
}