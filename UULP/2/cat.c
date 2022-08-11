/**
 * Created by iccy on 22-8-3.
 */

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#define PAGE 4 * 1024

int main(int argc, const char* argv[]) {
  int         fd = -1;
  struct stat st = {0};

  char buf[PAGE] = {0};

  if (2 != argc) {
    return -1;
  }

  if (-1 == stat(argv[1], &st)) {
    return -1;
  }

  if (!S_ISREG(st.st_mode)) {
    return -1;
  }

  fd = open(argv[1], O_RDONLY);
  if (-1 == fd) {
    return -1;
  }

  while (0 != read(fd, buf, PAGE)) {
    fprintf(stdout, "%s", buf);
    fflush(stdout);
  }

  return 0;
}