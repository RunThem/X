/**
 * Created by iccy on 22-8-18.
 *
 * ls - version 0.1
 * purpose list contents of directory or directories action if no args, use else
 * list files in args
 */

#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>

void do_ls(const char[]);

int main(int argc, const char *argv[]) {
  if (1 == argc) {
    do_ls(".");
  } else {
    while (--argc) {
      printf("%s: \n", *++argv);
      do_ls(*argv);
    }
  }

  return 0;
}

/**
 * list files in directory called dirname
 */
void do_ls(const char dirname[]) {
  DIR *dir_ptr = NULL;           /* the directory */
  struct dirent *direntp = NULL; /*each entry */

  if (NULL == (dir_ptr = opendir(dirname))) {
    fprintf(stderr, "ls: cannot open %s\b", dirname);
  } else {
    while (NULL != (direntp = readdir(dir_ptr))) {
      printf("%s\n", direntp->d_name);
    }

    closedir(dir_ptr);
  }
}
