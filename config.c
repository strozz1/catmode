

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char *find_kitty_image(void) {
  const char *home = getenv("HOME");
  if (!home)
    return NULL;

  static char path[PATH_MAX];

  const char *candidates[] = {
      ".config/catmode/background.png", ".config/catmode/background.jpg",
      ".config/catmode/bg.png", ".config/catmode/bg.jpg", NULL};

  for (int i = 0; candidates[i] != NULL; i++) {
    snprintf(path, sizeof(path), "%s/%s", home, candidates[i]);

    if (access(path, R_OK) == 0)
      return path;
  }

  return NULL;
}
