#ifndef ANIMATION_H
#define ANIMATION_H
#include <cairo/cairo.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  uint8_t *pixels;
  int width;
  int height;
  cairo_surface_t *surface;
} frame_t;

typedef struct animation_t {
  int fps;
  frame_t *frames;
  int total_frames;

  /*linear buffer containing all frames of the animation*/
  uint8_t *buffer;

  int width;
  int height;
  /*Index of the current animation frame from the buffer*/
  int idx;

  cairo_surface_t *surface;

} animation_t;

typedef struct {
  uint8_t *pixels;
  int sheet_width;
  int sheet_height;

  int frame_width;
  int frame_height;

  int cols;
  int total_frames;
} spritesheet_t;

animation_t *anim_from_spritesheet(const char *path, int fps);
frame_t *anim_next_frame(animation_t *anim);
void anim_destroy(animation_t *anim);
#endif
