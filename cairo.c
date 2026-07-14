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
  PangoLayout *layout = pango_cairo_create_layout(cr);
  pango_layout_set_text(layout, text, -1);

  PangoFontDescription *desc = pango_font_description_from_string(font_name);
  pango_font_description_set_absolute_size(desc, size * PANGO_SCALE);
  pango_layout_set_font_description(layout, desc);
  pango_font_description_free(desc);

  cairo_set_source_rgba(cr, 1, 1, 1, 1);
  // cairo_move_to(cr, x, y);

  PangoRectangle ink_rect, logical_rect;

  pango_layout_get_pixel_extents(layout, &ink_rect, &logical_rect);

  double draw_x = x - logical_rect.width / 2.0;
  double draw_y = y - logical_rect.height / 2.0;

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
