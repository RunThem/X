/**
 * Created by iccy on 22-8-1.
 *
 * ac - version 0.1
 * count user login time
 */

#include <fcntl.h>
#include <search.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utmp.h>

#define PW_FILE "/etc/passwd"
#define BUF_SIZE 512

typedef struct {
  char     user[UT_NAMESIZE];
  long int c_time;
  long int l_time;
} ut_s;

int main() {
  FILE*       pw_fd            = NULL;
  char        pw_buf[BUF_SIZE] = {0};
  int         count            = 0;
  ut_s*       ut               = NULL;
  struct utmp ut_buf           = {0};
  int         ut_fd            = -1;
  int         idx              = 0;
  int         i                = 0;

  /* count the number of accounts */
  pw_fd                        = fopen(PW_FILE, "r");
  if (NULL == pw_fd) {
    exit(1);
  }

  while ((fgets(pw_buf, BUF_SIZE, pw_fd)) != NULL) {
    if (NULL == strstr(pw_buf, "nologin")) {
      count++;
    }
  }

  /* alloc buf */
  ut = (ut_s*)calloc(count, sizeof(ut_s));
  if (NULL == ut) {
    exit(1);
  }

  ut_fd = open(WTMP_FILE, O_RDONLY);
  if (-1 == ut_fd) {
    exit(1);
  }

  while (true) {
    if (0 == read(ut_fd, &ut_buf, sizeof(struct utmp))) {
      break;
    }

    if (ut_buf.ut_type == USER_PROCESS) {
      for (i = 0; i < idx; i++) {
        if (!strncmp(ut_buf.ut_user, ut[i].user, UT_NAMESIZE)) {
          ut[i].l_time = ut_buf.ut_tv.tv_sec;
          break;
        }
      }

      if (i == idx) {
        sprintf(ut[i].user, "%s", ut_buf.ut_user, UT_NAMESIZE);
        ut[idx].l_time = ut_buf.ut_tv.tv_sec;
        idx++;
      }
    } else if (ut_buf.ut_type == DEAD_PROCESS) {
      for (i = 0; i < count; i++) {
        if (!strncmp(ut_buf.ut_user, ut[i].user, UT_NAMESIZE)) {
          ut[i].c_time += ut_buf.ut_tv.tv_sec - ut[i].l_time;
          break;
        }
      }
    }
  }

  for (i = 0; i < idx; i++) {
    printf("%s\t\t%ld\n", ut[i].user, ut[i].c_time);
  }

  fclose(pw_fd);
  close(ut_fd);

  return 0;
}
