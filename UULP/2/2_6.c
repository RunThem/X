/**
 * Created by iccy on 22-7-18.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#define BUF_SIZE 20

int main() {
  char buf[BUF_SIZE + 1];

  int fd1 = open("test", O_RDONLY);
  int fd2 = open("test", O_WRONLY);
  int fd3 = open("test", O_RDONLY);

  bzero(buf, BUF_SIZE + 1);

  read(fd1, buf, BUF_SIZE);
  printf("buf: %s\n", buf);

  write(fd2, "testing 123...", strlen("testing 123..."));

  read(fd3, buf, BUF_SIZE);
  printf("buf %s\n", buf);

  return 0;
}
