/**
 * Created by iccy on 22-7-16.
 *
 * buffer_test - test the effect of the system call read() on different buffer
 * sizes
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TEST_FILE "test.file"

char* buf;

void oop(const char* msg);

int main(int argc, const char* argv[]) {
  int fd;

  if (2 != argc) {
    oop("usage: buffer_test number");
  }

  buf = (char*)malloc(atoi(argv[1]));
  if (NULL == buf) {
    oop("malloc() failed");
  }

  fd = open(TEST_FILE, O_RDONLY);
  if (-1 == fd) {
    oop("open() failed");
  }

  while (read(fd, buf, atoi(argv[1]))) {
  }

  return 0;
}

void oop(const char* msg) {
  fprintf(stderr, "Error: %s\n", msg);
  exit(1);
}