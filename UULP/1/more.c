/**
 * Created by iccy on 22-7-3.
 *
 * more - version 0.6 of more
 * read and print 24 lines the pause for a few special commands
 * feature of version 0.2: read from /dev/tty for commands
 * feature of version 0.3: set /dev/tty to be unbuffered
 * feature of version 0.4: set /dev/tty to be echoless
 * feature of version 0.5: override "more?"
 * feature of version 0.6: dynamically get terminal size
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#define LINELEN 512

int page_len;

void do_more(FILE*);
int  see_more(FILE*);

int main(int argc, const char* argv[]) {
  FILE* fp = NULL;

  if (argc == 1) {
    do_more(stdin);
  } else {
    while (--argc) {
      if ((fp = fopen(*++argv, "r")) == NULL) {
        exit(1);
      }

      do_more(fp);
      fclose(fp);
    }
  }

  return 0;
}

/*
 * read page_len lines, then call see_more() for further instructions
 */
void do_more(FILE* fp) {
  char  line[LINELEN];
  int   num_of_lines = 0;
  int   reply;
  FILE* fp_tty = NULL;

  struct termios ctrl;
  struct winsize ws;

  fp_tty = fopen("/dev/tty", "r"); /* NEW: cmd stream */
  if (fp_tty == NULL) {            /* if open fails  */
    exit(1);                       /* no use in running */
  }

  tcgetattr(fileno(fp_tty), &ctrl);
  ctrl.c_lflag &=
      ~(ICANON | ECHO); /* turning off canonical mode makes input
                           unbuffered, also make the input echoless */
  tcsetattr(fileno(fp_tty), TCSANOW, &ctrl);

  ioctl(fileno(fp_tty), TIOCGWINSZ, &ws);

  page_len = ws.ws_row;

  while (fgets(line, LINELEN, fp)) { /* more input */
    if (num_of_lines == page_len) {  /* full screen? */
      reply = see_more(fp_tty);      /* y: ask user */
      if (reply == 0) {              /* n: done */
        break;
      }

      num_of_lines -= reply; /* reset count */

      fputs("\r       \r", stdout); /* override "more?" */
    }

    if (fputs(line, stdout) == EOF) { /* show line */
      exit(1);                        /* or die */
    }

    num_of_lines++; /* count it */
  }
}

/*
 * print message, wait for response, return # of lines to advance q means no,
 * space means yes, CR means one line
 * */
int see_more(FILE* cmd) {
  int c;
  printf("\033[7m more? \033[0m"); /* reverse on a vt100 */

  while ((c = getc(cmd)) != EOF) { /* get response  */
    switch (c) {
    case 'q': /* q -> N */
      return 0;
    case ' ':          /* ' ' => next page */
      return page_len; /* how many to show */
    case '\n':         /* Enter key => 1 line */
      return 1;
    }
  }

  return 0;
}