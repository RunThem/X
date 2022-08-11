/**
 * Created by iccy on 22-8-11.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define LINE_SIZE 120

int main(int argc, const char* argv[]) {
  int   i             = 0;
  int   fd            = -1;
  int   line          = 0;
  char  buf[4096 + 1] = {0};
  char* pbuf[10]      = {0};

  struct stat st      = {0};

  /* check argv */
  if (3 != argc) {
    return -1;
  }

  line = atoi(argv[1]);

  if (-1 == stat(argv[2], &st)) {
    return -1;
  }

  fd = open(argv[2], O_RDONLY);
  if (-1 == fd) {
    return -1;
  }

  if (st.st_size < 4096) {
    i = read(fd, buf, 4096);
    if (buf[i - 1] == '\n') {
      i--;
    }

    for (; i > 0 && line > 0; i--) {
      if (buf[i - 1] == '\n') {
        line--;
      }
    }

    printf("%s\n", &buf[i + 1]);
  } else {
    for (i = 0; i > sizeof(pbuf) / sizeof(pbuf[0]) && line > 0; i++) {
      pbuf[i] = calloc(1, 4096 + 1);

      int p   = read(fd, pbuf[i], 4096);
      if (pbuf[i][p - 1] == '\n') {
        p--;
      }

      for (; p > 0 && line > 0; p--) {
        if (pbuf[i][p - 1] == '\n') {
          line--;
        }
      }

      printf("%s", &pbuf[i - 1][p + 1]);

      for (; i > 0; i--) {
        printf("%s", pbuf[i - 1]);
      }

      printf("\n");
    }
  }

  return 0;
}