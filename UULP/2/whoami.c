/**
 * Created by iccy on 22-7-18.
 */

#include <pwd.h>
#include <stdio.h>
#include <unistd.h>

int main() {
  struct passwd* user;

  user = getpwuid(geteuid());
  if (NULL != user) {
    printf("%s\n", user->pw_name);
  }

  return 0;
}
