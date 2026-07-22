#include "overlay.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char *find_kitty_spritesheet(void) {
  static char path[PATH_MAX];
  static char *default_path = "/usr/local/share/catmode/catsheet.png";

  const char *home = getenv("HOME");

  const char *candidates[] = {"catmode/catsheet.png", "catmode/spritesheet.png",
                              NULL};

  printf("Looking for spritesheet image:\n");

  /* ~/.config */
  if (home) {
    for (int i = 0; candidates[i]; i++) {
      snprintf(path, sizeof(path), "%s/.config/%s", home, candidates[i]);

      printf("Candidate: %s\n", path);

      if (access(path, R_OK) == 0)
        return path;
    }
  }

  if (access(default_path, R_OK) == 0)
    return default_path;

  return NULL;
}
char *find_kitty_image(void) {
  static char path[PATH_MAX];
  static char *default_path = "/usr/local/share/catmode/catmode.png";

  const char *home = getenv("HOME");

  const char *candidates[] = {"catmode/background.png",
                              "catmode/background.jpg", "catmode/bg.png",
                              "catmode/bg.jpg", NULL};

  printf("Looking for background image:\n");

  /* ~/.config */
  if (home) {
    for (int i = 0; candidates[i]; i++) {
      snprintf(path, sizeof(path), "%s/.config/%s", home, candidates[i]);

      printf("Candidate: %s\n", path);

      if (access(path, R_OK) == 0)
        return path;
    }
  }

  if (access(default_path, R_OK) == 0)
    return default_path;

  return NULL;
}

int main(void) {
  overlay_t *ov = overlay_create_on_monitor(0);
  //ov->image_path = find_kitty_image();
  ov->spritesheet_path = find_kitty_spritesheet();

  //fprintf(stderr, "Using bg: '%s'\n", ov->image_path);
  fprintf(stderr, "Using animation: '%s'\n", ov->spritesheet_path);
  if (!ov) {
    fprintf(stderr, "Couldn't create overlay\n");
    return 1;
  }

  overlay_run(ov);

  overlay_destroy(ov);

  return 0;
}
