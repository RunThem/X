/**
 * Created by iccy on 22-7-4.
 *
 * who.c - version 0.2
 * open, read UTMP file, and show results
 * feature of version 0.2: determine if the user is already logged in
 * feature of version 0.3: format time to make it readable
 * feature of version 0.4: buffers input(using utmplib)
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <utmp.h>

#define SHOWHOST /* include remote machine on output */

void show_info(struct utmp* utbufp);

extern int          utmp_open(char* filename);
extern struct utmp* utmp_next();
extern void         utmp_close();

int main() {
  struct utmp* utbufp;

  if (-1 == utmp_open(UTMP_FILE)) {
    perror(UTMP_FILE); /* UTMP_FILE is in utmp.h */
    exit(1);
  }

  while (NULL != (utbufp = utmp_next())) {
    show_info(utbufp);
  }

  utmp_close();

  return 0;
}

/*
 * show info()
 * displays contents of the utmp struct in human readable from * note * these
 * sizes should not be hardwired
 */
void show_info(struct utmp* utbufp) {
  time_t     nowtime;
  struct tm* nowtm;
  char       tmbuf[64];

  printf("%-8.8s ", utbufp->ut_name); /* logname */
  printf("%-8.8s ", utbufp->ut_line); /*  a space */

  nowtime = utbufp->ut_xtime;
  nowtm   = localtime(&nowtime);
  strftime(tmbuf, sizeof(tmbuf), "%Y-%m-%d %H:%M", nowtm);
  printf("%s ", tmbuf); /* login time */

#ifdef SHOWHOST
  printf("(%s)", utbufp->ut_host); /* host */
#endif

  printf("\n");
}
