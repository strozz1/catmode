#include "cairo.h"
#include <cairo/cairo-xcb.h>
#include <cairo/cairo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <xcb/xcb.h>

typedef struct {
  int16_t x, y;
  uint16_t w, h;
  void (*on_click)(void *userdata);
  void *userdata;
} clickable_t;

typedef struct {
  int16_t x; // top left coords from screen
  int16_t y;
  int16_t width;
  int16_t height;
} monitor_t;

typedef struct {
  xcb_connection_t *conn;
  xcb_screen_t *screen;
  xcb_visualtype_t *visual; // for 32bit colors
  xcb_colormap_t colormap;
  xcb_window_t win;

  int16_t x, y;           // pos
  uint16_t width, height; // size

  volatile sig_atomic_t running;
  int destroyed_externally;

  clickable_t *clickables;
  size_t n_clickables;
  size_t clickables_cap;

  int current_hover; //-1 if none

  // cairo
  cairo_surface_t *cr_surface;
  cairo_t *cr;
  monitor_t *monitors;
  int mon_no;
  char *image_path;
} overlay_t;

overlay_t *overlay_create(int16_t x, int16_t y, uint16_t w, uint16_t h);
overlay_t *overlay_create_on_monitor(int monitor);

void overlay_run(overlay_t *ov);
void overlay_destroy(overlay_t *ov);

int overlay_get_monitor(overlay_t *ov, int id, monitor_t *mon);
