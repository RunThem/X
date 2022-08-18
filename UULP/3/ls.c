/**
 * Created by iccy on 22-8-18.
 *
 * ls - version 0.1
 * purpose list contents of directory or directories action if no args, use else
 * list files in args
 * note uses stat and pwd.h and grp.h
 */

#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

void  do_ls(const char[]);
void  do_stat(char*);
void  show_file_info(char*, struct stat*);
void  mode_to_letters(int, char[]);
char* uid_to_name(uid_t);
char* gid_to_name(gid_t);

int main(int argc, const char* argv[]) {
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

/*
 * list files in directory called dirname
 */
void do_ls(const char dirname[]) {
  DIR*           dir_ptr  = NULL; /* the directory */
  struct dirent* dirent_p = NULL; /*each entry */

  if (NULL == (dir_ptr = opendir(dirname))) {
    fprintf(stderr, "ls: cannot open %s\b", dirname);
  } else {
    while (NULL != (dirent_p = readdir(dir_ptr))) {
      do_stat(dirent_p->d_name);
    }

    closedir(dir_ptr);
  }
}

void do_stat(char* filename) {
  struct stat info;

  if (-1 == stat(filename, &info)) { /* cannot stat */
    perror(filename);                /* say why */
  } else {
    show_file_info(filename, &info); /* else show info */
  }
}

/*
 * display the info about filename. The info is stored in struct at * info_p
 */
void show_file_info(char* filename, struct stat* info_p) {
  char mode_str[11];

  mode_to_letters(info_p->st_mode, mode_str);

  printf("%s", mode_str);
  printf("%4d ", (int)info_p->st_nlink);
  printf("%-8s", uid_to_name(info_p->st_uid));
  printf("%-8s", gid_to_name(info_p->st_gid));
  printf("%8ld ", (long)info_p->st_size);
  printf("%.12s ", 4 + ctime(&info_p->st_mtime));
  printf("%s\n", filename);
}

/*
 * this function takes a mode value and a char array and puts into the char
 * array the file type and the nine letters that correspond to the bits in mode.
 * NOTE: it does not code setuid, setgid, and sticky codes
 */
void mode_to_letters(int mode, char str[]) {
  strncpy(str, "----------", strlen("----------")); /* default = no perms */

  if (S_ISDIR(mode))
    str[0] = 'd'; /* directory? */
  if (S_ISCHR(mode))
    str[0] = 'c'; /* char devices */
  if (S_ISBLK(mode))
    str[0] = 'b'; /* block device */

  /* 3 bits for user */
  if (mode & S_IRUSR)
    str[1] = 'r';
  if (mode & S_IWUSR)
    str[2] = 'w';
  if (mode & S_IXUSR)
    str[3] = 'x';

  /* 3 bits for group */
  if (mode & S_IRGRP)
    str[4] = 'r';
  if (mode & S_IWGRP)
    str[5] = 'w';
  if (mode & S_IXGRP)
    str[6] = 'x';

  /* 3 bits for other */
  if (mode & S_IROTH)
    str[7] = 'r';
  if (mode & S_IWOTH)
    str[8] = 'w';
  if (mode & S_IXOTH)
    str[9] = 'x';
}

/*
 * returns pointer to username associated with uid, uses getpw()
 */
char* uid_to_name(uid_t uid) {
  struct passwd* pw_ptr;
  static char    num_str[10];

  if (NULL == (pw_ptr = getpwuid(uid))) {
    snprintf(num_str, sizeof(num_str), "%d", uid);
    return num_str;
  }

  return pw_ptr->pw_name;
}

/*
 * returns pointer to group number gid. used getgrgid(3)
 */
char* gid_to_name(gid_t gid) {
  struct group* grp_ptr;
  static char   num_str[10];

  if (NULL == (grp_ptr = getgrgid(gid))) {
    snprintf(num_str, sizeof(num_str), "%d", gid);
    return num_str;
  }

  return grp_ptr->gr_name;
}
