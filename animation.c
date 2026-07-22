#include "animation.h"
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

frame_t *anim_next_frame(animation_t *anim) {
  frame_t *frame = &anim->frames[anim->idx];

  anim->idx = (anim->idx + 1) % anim->total_frames;

  return frame;
}

uint8_t *spritesheet_to_linear(const spritesheet_t *ss) {
  int frame_bytes = ss->frame_width * ss->frame_height * 4;
  uint8_t *linear = malloc(ss->total_frames * frame_bytes);
  if (!linear) {
    perror("malloc");
    exit(1);
  }

  int cols = ss->sheet_width / ss->frame_width;

  for (int frame = 0; frame < ss->total_frames; frame++) {
    int col = frame % cols;
    int row = frame / cols;

    uint8_t *dst = linear + frame * frame_bytes;

    for (int y = 0; y < ss->frame_height; y++) {
      const uint8_t *src =
          ss->pixels + ((row * ss->frame_height + y) * ss->sheet_width +
                        col * ss->frame_width) *
                           4;

      memcpy(dst + y * ss->frame_width * 4, src, ss->frame_width * 4);
    }
  }

  return linear;
}

frame_t *spritesheet_to_frames(const spritesheet_t *ss) {
  int frame_bytes = ss->frame_width * ss->frame_height * 4;

  frame_t *frames = malloc(sizeof(frame_t) * ss->total_frames);
  if (!frames) {
    perror("malloc");
    exit(1);
  }

  int cols = ss->sheet_width / ss->frame_width;

  for (int frame = 0; frame < ss->total_frames; frame++) {
    int col = frame % cols;
    int row = frame / cols;

    frames[frame].width = ss->frame_width;
    frames[frame].height = ss->frame_height;

    frames[frame].pixels = malloc(frame_bytes);
    if (!frames[frame].pixels) {
      perror("malloc");
      exit(1);
    }

    for (int y = 0; y < ss->frame_height; y++) {
      const uint8_t *src =
          ss->pixels + ((row * ss->frame_height + y) * ss->sheet_width +
                        col * ss->frame_width) *
                           4;

      uint8_t *dst = frames[frame].pixels + y * ss->frame_width * 4;

      memcpy(dst, src, ss->frame_width * 4);
    }

    frames[frame].surface = cairo_image_surface_create_for_data(
        frames[frame].pixels, CAIRO_FORMAT_ARGB32, frames[frame].width,
        frames[frame].height, frames[frame].width * 4);
  }

  return frames;
}

uint8_t *convert_rgba_to_argb32(uint8_t *src, int width, int height) {
  uint8_t *dst = malloc(width * height * 4);

  for (int i = 0; i < width * height; i++) {
    uint8_t r = src[i * 4 + 0];
    uint8_t g = src[i * 4 + 1];
    uint8_t b = src[i * 4 + 2];
    uint8_t a = src[i * 4 + 3];

    dst[i * 4 + 0] = (b * a) / 255;
    dst[i * 4 + 1] = (g * a) / 255;
    dst[i * 4 + 2] = (r * a) / 255;
    dst[i * 4 + 3] = a;
  }

  return dst;
}

spritesheet_t *anim_load_spritesheet(const char *path, int frame_width,
                                     int frame_height) {
  int width, height, channels;
  uint8_t *pixels = stbi_load(path, &width, &height, &channels, 4);
  if (!pixels) {
    fprintf(stderr, "[Error] %s: file: %s\n", stbi_failure_reason(),path);
    exit(1);
  }
  if (width % frame_width || height % frame_height) {
    fprintf(stderr, "Invalid spritesheet dimensions\n");
    stbi_image_free(pixels);
    exit(1);
  }

  int cols = width / frame_width;
  int rows = height / frame_height;
  int frames = cols * rows;

  spritesheet_t *ss = malloc(sizeof(spritesheet_t));
  if (ss == NULL) {
    perror("malloc");
    exit(1);
  }
  uint8_t *data = convert_rgba_to_argb32(pixels, width, height);
  stbi_image_free(pixels);
  ss->pixels = data;
  ss->frame_width = frame_width;
  ss->frame_height = frame_height;
  ss->total_frames = frames;
  ss->sheet_width = width;
  ss->sheet_height = height;
  return ss;
}

animation_t *anim_from_spritesheet(const char *path, int fps) {
  spritesheet_t *ss = anim_load_spritesheet(path, 384, 384);

  animation_t *anim = malloc(sizeof(animation_t));

  if (!anim) {
    perror("malloc");
    exit(1);
  }

  anim->fps = fps;
  anim->frames = spritesheet_to_frames(ss);
  anim->total_frames = ss->total_frames;
  anim->idx = 0;

  free(ss->pixels);
  free(ss);

  return anim;
}

void anim_destroy(animation_t *anim) {
  for (int i = 0; i < anim->total_frames; i++) {
    cairo_surface_destroy(anim->frames[i].surface);
    free(anim->frames[i].pixels);
  }

  free(anim->frames);
  free(anim);
}
