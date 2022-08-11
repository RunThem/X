/**
 * Created by iccy on 22-8-3.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define PAGE 4 * 1024

int main(int argc, const char* argv[]) {
  FILE* fd         = NULL;
  int   lines      = 10;
  char  buf[PAGE]  = {0};

  const char* file = argv[1];
  struct stat st   = {0};

  if (3 == argc) {
    lines = atoi(argv[1]);
    file  = argv[2];
  }

  if (-1 == stat(file, &st)) {
    return -1;
  }

  if (!S_ISREG(st.st_mode)) {
    return -1;
  }

  fd = fopen(file, "r");
  if (NULL == fd) {
    return -1;
  }

  if (2 == argc) {
    while (0 != fread(buf, 1, PAGE, fd)) {
      fprintf(stdout, "%s", buf);
    }
  } else {
    while (lines) {
      fgets(buf, PAGE, fd);
      fprintf(stdout, "%s", buf);
      lines--;
    }
  }

  return 0;
}