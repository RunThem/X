/**
 * Created by iccy on 22-7-17.
 *
 * utmplib.c - function to buffer reads from umtp file
 *
 *      function are
 *          utmp_open(filename)     - open file
 *              return -1 on error
 *          utmp_next()             - return pointer to next struct
 *              return NULL on eof
 *          utmp_close()            - close file
 *
 *      reads NRECS per read and then doles them out from the buffer
 */

#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <utmp.h>

#define NRECS 16
#define NULLUT ((struct utmp*)NULL)
#define UTSIZE (sizeof(struct utmp))

static char utmpbuf[NRECS * UTSIZE]; /* storage */
static int  num_recs;                /* num stored */
static int  cur_rec;                 /* next to go */
static int  fd_utmp = -1;            /* read from */

static int utmp_reload();

int utmp_open(char* filename) {
  fd_utmp = open(filename, O_RDONLY); /* open it */
  cur_rec = num_recs = 0;             /* no recs yet */
  return fd_utmp;                     /* report */
}

struct utmp* utmp_next() {
  struct utmp* recp = NULL;
  if (-1 == fd_utmp) /* error? */
  {
    return NULLUT;
  }

  if (cur_rec == num_recs && 0 == utmp_reload()) /* anymore */
  {
    return NULLUT;
  }

  /* get address of next record */
  do {
    recp = (struct utmp*)&utmpbuf[cur_rec * UTSIZE];
    cur_rec++;
  } while (cur_rec < num_recs && USER_PROCESS != recp->ut_type);

  if (USER_PROCESS != recp->ut_type) {
    return NULLUT;
  }

  return recp;
}

/*
 * read next bunch of records into buffer.
 */
static int utmp_reload() {
  int amt_read; /* read them in */

  amt_read = read(fd_utmp, utmpbuf, NRECS * UTSIZE); /* how many did we get? */
  num_recs = amt_read / UTSIZE;
  cur_rec  = 0;

  return num_recs;
}

void utmp_close() {
  if (-1 != fd_utmp) {
    close(fd_utmp); /* close */
  }
}