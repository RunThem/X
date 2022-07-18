/**
 * Created by iccy on 22-7-18.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 20

int main(int argc, const char* argv[]) {

  char buf[BUF_SIZE + 1];

  if (2 != argc) {
    return -1;
  }

  memset(buf, 1, BUF_SIZE);

  int fd = open(argv[1], O_RDWR);

  lseek(fd, 100, SEEK_END);
  int n = read(fd, buf, BUF_SIZE);
  printf("n: %d\n", n);
  for (ssize_t i = 0; i < BUF_SIZE; i++) {
    printf("0x%x ", buf[i]);
  }

  printf("\n");

  lseek(fd, 2000, SEEK_END);
  write(fd, "hello", strlen("hello"));

  n = read(fd, buf, BUF_SIZE);
  printf("n: %d, buf %s\n", n, buf);

  return 0;
}