#include "cairo.h"

void draw_png_centered(cairo_t *cr, const char *path, double screen_w,
                       double screen_h) {
  cairo_surface_t *img = cairo_image_surface_create_from_png(path);

  if (cairo_surface_status(img) != CAIRO_STATUS_SUCCESS) {
    fprintf(stderr, "Error opening image\n");
    cairo_surface_destroy(img);
    return;
  }
  int width = cairo_image_surface_get_width(img);
  int height = cairo_image_surface_get_height(img);
  int w = (screen_w - width) / 2;
  int h = (screen_h - height) / 2;

  cairo_set_source_surface(cr, img, w, h);
  cairo_paint(cr);

  cairo_surface_destroy(img);
}

void draw_anim(cairo_t *cr, animation_t *anim, double screen_w,
               double screen_h) {
  frame_t *frame = &anim->frames[anim->idx];

  cairo_surface_mark_dirty(frame->surface);

  int x = (screen_w - frame->width) / 2;
  int y = (screen_h - frame->height) / 2;

  cairo_set_source_surface(cr, frame->surface, x, y);

  cairo_paint(cr);
}

/*
#include <librsvg/rsvg.h>

void draw_svg(cairo_t *cr, const char *path, double x, double y, double w,
double h) { GError *error = NULL; RsvgHandle *handle =
rsvg_handle_new_from_file(path, &error); if (!handle) { g_error_free(error);
        return;
    }

    cairo_save(cr);
    cairo_translate(cr, x, y);

    RsvgRectangle viewport = { 0, 0, w, h };
    rsvg_handle_render_document(handle, cr, &viewport, &error);

    cairo_restore(cr);
    g_object_unref(handle);
}
*/

void draw_text(cairo_t *cr, const char *text, double x, double y, int size,
               font_type_t font) {
  const char *font_name;
  switch (font) {
  case FONT_REGULAR:
    font_name = "Inter";
    break;
  case FONT_MEDIUM:
    font_name = "Inter Medium";
    break;
  case FONT_BOLD:
    font_name = "Inter Bold";
    break;
  case FONT_ITALIC:
    font_name = "Inter Italic";
    break;
  case FONT_MONO:
    font_name = "JetBrains Mono";
    break;
  default:
    font_name = "Sans";
    break;
  }

  cairo_font_options_t *font_options = cairo_font_options_create();
  cairo_font_options_set_antialias(font_options, CAIRO_ANTIALIAS_GRAY);
  cairo_font_options_set_hint_style(font_options, CAIRO_HINT_STYLE_SLIGHT);
  cairo_font_options_set_hint_metrics(font_options, CAIRO_HINT_METRICS_ON);
  cairo_set_font_options(cr, font_options);
  cairo_font_options_destroy(font_options);

  PangoLayout *layout = pango_cairo_create_layout(cr);
  pango_layout_set_text(layout, text, -1);

  PangoFontDescription *desc = pango_font_description_from_string(font_name);
  pango_font_description_set_absolute_size(desc, size * PANGO_SCALE);
  pango_layout_set_font_description(layout, desc);
  pango_font_description_free(desc);

  PangoAttrList *attrs = pango_attr_list_new();
  pango_attr_list_insert(attrs, pango_attr_letter_spacing_new(20));
  pango_layout_set_attributes(layout, attrs);
  pango_attr_list_unref(attrs);

  PangoRectangle ink_rect, logical_rect;
  pango_layout_get_pixel_extents(layout, &ink_rect, &logical_rect);
  double draw_x = round(x - logical_rect.width / 2.0);
  double draw_y = round(y - logical_rect.height / 2.0);

  cairo_save(cr);
  cairo_set_source_rgba(cr, 0, 0, 0, 0.35);
  cairo_move_to(cr, draw_x + 1, draw_y + 1);
  pango_cairo_show_layout(cr, layout);
  cairo_restore(cr);

  cairo_set_source_rgba(cr, 0.94, 0.95, 0.97, 1);
  cairo_move_to(cr, draw_x, draw_y);
  pango_cairo_show_layout(cr, layout);

  g_object_unref(layout);
}

void draw_rounded_rectangle(cairo_t *cr, double x, double y, double width,
                            double height, double radius) {
  if (radius > width / 2.0)
    radius = width / 2.0;

  if (radius > height / 2.0)
    radius = height / 2.0;

  cairo_new_path(cr);

  cairo_arc(cr, x + width - radius, y + radius, radius, -G_PI / 2.0, 0);

  cairo_arc(cr, x + width - radius, y + height - radius, radius, 0, G_PI / 2.0);

  cairo_arc(cr, x + radius, y + height - radius, radius, G_PI / 2.0, G_PI);

  cairo_arc(cr, x + radius, y + radius, radius, G_PI, 3.0 * G_PI / 2.0);

  cairo_close_path(cr);
}
