#include <cairo/cairo.h>
#include <math.h>
#include <pango/pangocairo.h>
#include <stdio.h>
#include "animation.h"

typedef enum {
  FONT_REGULAR,
  FONT_MEDIUM,
  FONT_BOLD,
  FONT_ITALIC,
  FONT_MONO
} font_type_t;

void draw_text(cairo_t *cr, const char *text, double x, double y, int size,
               font_type_t font);
void draw_png_centered(cairo_t *cr, const char *path, double x, double y);

void draw_anim(cairo_t *cr, animation_t *anim, double screen_w,
               double screen_h);
void draw_rounded_rectangle(cairo_t *cr, double x, double y, double width,
                            double height, double radius);
