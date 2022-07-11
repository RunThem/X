/**
 * Created by iccy on 22-7-4.
 *
 * who.c - a first version of the who program open, read UTMP file, and show
 * results
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <utmp.h>

#define SHOWHOST /* include remote machine on output */

void    show_info(struct utmp* utbufp);
ssize_t format_timeval(struct timeval* tv, char* buf, size_t sz);

int main() {
  struct utmp current_record; /* read info into here */
  int         utmpfd;         /* read from this descriptor */
  int         reclen = sizeof(struct utmp);

  if ((utmpfd = open(UTMP_FILE, O_RDONLY)) == -1) {
    perror(UTMP_FILE); /* UTMP_FILE is in utmp.h */
    exit(1);
  }

  while (read(utmpfd, &current_record, reclen) == reclen) {
    show_info(&current_record);
  }

  close(utmpfd);

  return 0;
}

/*
 * show info()
 * displays contents of the utmp struct in human readable from * note * these
 * sizes should not be hardwired
 */
void show_info(struct utmp* utbufp) {
  char buf[28] = {0};

  printf("%-8.8s ", utbufp->ut_name); /* logname */
  printf("%-8.8s ", utbufp->ut_line); /*  a space */

  printf("%d ", utbufp->ut_xtime); /* login time */

#ifdef SHOWHOST
  printf("(%s)", utbufp->ut_host); /* host */
#endif

  printf("\n");
}
