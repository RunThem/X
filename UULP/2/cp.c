/**
 * Created by iccy on 22-7-11.
 *
 * cp - version 0.1
 * uses read and write with tunable buffer size
 *      usage: cp src dest
 */

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFERSIZE 4096
#define COPYMODE 0644

void oops(char*, const char*);

static bool confirm;

int main(int argc, const char* argv[]) {
  int  in_fd, out_fd, n_chars;
  char buf[BUFFERSIZE];
  int  src_idx = 1;
  int  dst_idx = 2;

  /* check args */
  if (3 > argc) {
    fprintf(stderr, "usage: %s source destination\n", argv[0]);
    exit(1);
  }

  if (!strncmp(argv[1], "-i", strlen("-i"))) {
    confirm = true;
    src_idx++;
    dst_idx++;
  }

  if (confirm && (0 == access(argv[dst_idx], F_OK))) {
    fprintf(stderr, "file %s exited.\nwhether to cover(y/n):", argv[dst_idx]);
    if ('n' == getchar()) {
      exit(0);
    }
  }

  /* open file */
  if (-1 == (in_fd = open(argv[src_idx], O_RDONLY))) {
    oops("Cannot open ", argv[src_idx]);
  }

  if (-1 == (out_fd = creat(argv[dst_idx], COPYMODE))) {
    oops("Cannot creat ", argv[dst_idx]);
  }

  /* copy file */
  while (0 < (n_chars = read(in_fd, buf, BUFFERSIZE))) {
    if (n_chars != write(out_fd, buf, n_chars)) {
      oops("Write error to ", argv[dst_idx]);
    }
  }

  if (-1 == n_chars) {
    oops("Read error from ", argv[1]);
  }

  /* close files */
  if (-1 == close(in_fd) || -1 == close(out_fd)) {
    oops("Error closing files", "");
  }
}

void oops(char* s1, const char* s2) {
  fprintf(stderr, "Error, %s", s1);
  perror(s2);
  exit(1);
}