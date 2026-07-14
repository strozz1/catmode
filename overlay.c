#include "overlay.h"
#include <xcb/randr.h>

/*Obtain the ARGB visual (@p `xcb_visualtype_t`) for TRUE COLOR(32 bits).
 */
xcb_visualtype_t *find_argb_visual(xcb_screen_t *screen) {
  xcb_depth_iterator_t depth_iter = xcb_screen_allowed_depths_iterator(screen);
  for (; depth_iter.rem; xcb_depth_next(&depth_iter)) {
    if (depth_iter.data->depth != 32)
      continue;
    xcb_visualtype_iterator_t visual_iter =
        xcb_depth_visuals_iterator(depth_iter.data);
    for (; visual_iter.rem; xcb_visualtype_next(&visual_iter)) {
      if (visual_iter.data->_class == XCB_VISUAL_CLASS_TRUE_COLOR)
        return visual_iter.data;
    }
  }
  return NULL;
}

overlay_t *setup() {
  overlay_t *ov = calloc(1, sizeof(overlay_t));

  ov->conn = xcb_connect(NULL, NULL);
  if (xcb_connection_has_error(ov->conn)) {
    free(ov);
    fprintf(stderr, "Error stablishing connection to X Server\n");
    return NULL;
  }

  const xcb_setup_t *setup = xcb_get_setup(ov->conn);
  ov->screen = xcb_setup_roots_iterator(setup).data;
  ov->visual = find_argb_visual(ov->screen);

  ov->colormap = xcb_generate_id(ov->conn);
  xcb_create_colormap(ov->conn, XCB_COLORMAP_ALLOC_NONE, ov->colormap,
                      ov->screen->root, ov->visual->visual_id);

  ov->win = xcb_generate_id(ov->conn);

  ov->current_hover = -1;
  return ov;
}

int overlay_get_monitor_count(overlay_t *ov) {
  xcb_randr_get_screen_resources_current_cookie_t cookie =
      xcb_randr_get_screen_resources_current(ov->conn, ov->screen->root);

  xcb_randr_get_screen_resources_current_reply_t *res =
      xcb_randr_get_screen_resources_current_reply(ov->conn, cookie, NULL);

  if (!res)
    return 0;

  int len = xcb_randr_get_screen_resources_current_outputs_length(res);

  xcb_randr_output_t *outputs =
      xcb_randr_get_screen_resources_current_outputs(res);

  int count = 0;

  for (int i = 0; i < len; i++) {
    xcb_randr_get_output_info_cookie_t ocookie =
        xcb_randr_get_output_info(ov->conn, outputs[i], XCB_CURRENT_TIME);

    xcb_randr_get_output_info_reply_t *oinfo =
        xcb_randr_get_output_info_reply(ov->conn, ocookie, NULL);

    if (oinfo) {
      if (oinfo->connection == XCB_RANDR_CONNECTION_CONNECTED &&
          oinfo->crtc != XCB_NONE)
        count++;

      free(oinfo);
    }
  }

  free(res);
  ov->mon_no = count;

  return count;
}

void overlay_setup_surface(overlay_t *ov) {
  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXEL |
                  XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK |
                  XCB_CW_COLORMAP;
  uint32_t values[5] = {0, 0, 1,
                        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS |
                            XCB_EVENT_MASK_BUTTON_PRESS |
                            XCB_EVENT_MASK_POINTER_MOTION |
                            XCB_EVENT_MASK_STRUCTURE_NOTIFY,
                        ov->colormap};

  xcb_create_window(ov->conn, 32, ov->win, ov->screen->root, ov->x, ov->y,
                    ov->width, ov->height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    ov->visual->visual_id, mask, values);

  xcb_map_window(ov->conn, ov->win);
  xcb_set_input_focus(ov->conn, XCB_INPUT_FOCUS_POINTER_ROOT, ov->win,
                      XCB_CURRENT_TIME);

  xcb_flush(ov->conn);
  ov->cr_surface = cairo_xcb_surface_create(ov->conn, ov->win, ov->visual,
                                            ov->width, ov->height);
  ov->cr = cairo_create(ov->cr_surface);
  xcb_flush(ov->conn);
}
/*
 *Creates the overlay with the position and size.
 */
overlay_t *overlay_create(int16_t x, int16_t y, uint16_t w, uint16_t h) {
  overlay_t *ov = setup();
  ov->x = x;
  ov->y = y;
  ov->width = w;
  ov->height = h;
  ov->running = 1;
  overlay_setup_surface(ov);

  return ov;
}

overlay_t *overlay_create_on_monitor(int monitor) {
  overlay_t *ov = setup();
  overlay_get_monitor_count(ov);
  ov->monitors = calloc(ov->mon_no, sizeof(monitor_t));
  for (int i = 0; i < ov->mon_no; i++) {
    overlay_get_monitor(ov, i, &ov->monitors[i]);
    monitor_t *m = &ov->monitors[i];
    fprintf(stderr, "Dectected Monitor %d: %dx%d @ (%d,%d)\n", i, m->width,
            m->height, m->x, m->y);
  }
  monitor_t *m = &ov->monitors[monitor];
  ov->x = m->x;
  ov->y = m->y;
  ov->width = m->width;
  ov->height = m->height;
  ov->running = 1;
  overlay_setup_surface(ov);
  return ov;
}

int overlay_get_monitor(overlay_t *ov, int id, monitor_t *monitor) {
  xcb_randr_get_screen_resources_current_cookie_t cookie =
      xcb_randr_get_screen_resources_current(ov->conn, ov->screen->root);

  xcb_randr_get_screen_resources_current_reply_t *res =
      xcb_randr_get_screen_resources_current_reply(ov->conn, cookie, NULL);

  if (!res)
    return 0;

  int len = xcb_randr_get_screen_resources_current_outputs_length(res);

  if (id >= len) {
    free(res);
    return 0;
  }

  xcb_randr_output_t *outputs =
      xcb_randr_get_screen_resources_current_outputs(res);

  xcb_randr_get_output_info_cookie_t ocookie =
      xcb_randr_get_output_info(ov->conn, outputs[id], XCB_CURRENT_TIME);

  xcb_randr_get_output_info_reply_t *oinfo =
      xcb_randr_get_output_info_reply(ov->conn, ocookie, NULL);

  if (!oinfo || oinfo->crtc == XCB_NONE) {
    free(oinfo);
    free(res);
    return 0;
  }

  xcb_randr_get_crtc_info_cookie_t ccookie =
      xcb_randr_get_crtc_info(ov->conn, oinfo->crtc, XCB_CURRENT_TIME);

  xcb_randr_get_crtc_info_reply_t *cinfo =
      xcb_randr_get_crtc_info_reply(ov->conn, ccookie, NULL);

  if (!cinfo) {
    free(oinfo);
    free(res);
    return 0;
  }

  monitor->x = cinfo->x;
  monitor->y = cinfo->y;
  monitor->width = cinfo->width;
  monitor->height = cinfo->height;

  free(cinfo);
  free(oinfo);
  free(res);

  return 1;
}

void old_overlay_repaint(overlay_t *ov) {

  xcb_gcontext_t gc = xcb_generate_id(ov->conn);
  uint32_t mask = XCB_GC_FOREGROUND;
  uint32_t values[1] = {0x80FF0000};
  xcb_create_gc(ov->conn, gc, ov->win, mask, values);

  xcb_rectangle_t rect = {0, 0, ov->width, ov->height};
  xcb_poly_fill_rectangle(ov->conn, ov->win, gc, 1, &rect);

  xcb_free_gc(ov->conn, gc);
  xcb_flush(ov->conn);
}

void overlay_repaint(overlay_t *ov) {
  cairo_t *cr = ov->cr;

  cairo_save(cr);
  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_rgba(cr, 0, 0, 0, 0);
  cairo_paint(cr);
  cairo_restore(cr);

  // BG
  cairo_set_source_rgba(cr, 24.0 / 255.0, 24.0 / 255.0, 27.0 / 255.0, 0.82);

  draw_rounded_rectangle(cr, 0, 0, ov->width, ov->height, 18);
  cairo_fill(cr);
  draw_png_centered(cr, ov->image_path, ov->width, ov->height);

  draw_text(cr, "Cat Mode Enabled", ov->width * 0.5, ov->height * 0.7, 58,
            FONT_BOLD);
  draw_text(cr, "Your keyboard is now protected", ov->width * 0.5,
            ov->height * 0.7 + 70, 26, FONT_REGULAR);
  draw_text(cr, "from curious paws.", ov->width * 0.5, ov->height * 0.7 + 95,
            26, FONT_REGULAR);

  draw_text(cr, "Control-F to resume", ov->width * 0.5, ov->height * 0.90, 26,
            FONT_REGULAR);
  cairo_surface_flush(ov->cr_surface);
  xcb_flush(ov->conn);
}

void overlay_handle_click(overlay_t *ov, int16_t x, int16_t y, uint8_t button) {
  if (button != 1)
    return; 

  for (size_t i = 0; i < ov->n_clickables; i++) {
    clickable_t *c = &ov->clickables[i];
    if (x >= c->x && x < c->x + c->w && y >= c->y && y < c->y + c->h) {
      c->on_click(c->userdata);
      return;
    }
  }
}

void overlay_handle_hover(overlay_t *ov, int16_t x, int16_t y) {
  int new_hover = -1;

  for (size_t i = 0; i < ov->n_clickables; i++) {
    clickable_t *c = &ov->clickables[i];
    if (x >= c->x && x < c->x + c->w && y >= c->y && y < c->y + c->h) {
      new_hover = (int)i;
      break;
    }
  }

  if (new_hover != ov->current_hover) {
    ov->current_hover = new_hover;
    overlay_repaint(ov); 
  }
}

void overlay_destroy(overlay_t *ov) {
  if (!ov)
    return;
  cairo_destroy(ov->cr);
  cairo_surface_destroy(ov->cr_surface);
  if (!ov->destroyed_externally)
    xcb_destroy_window(ov->conn, ov->win);
  xcb_free_colormap(ov->conn, ov->colormap);
  xcb_flush(ov->conn);
  xcb_disconnect(ov->conn);
  free(ov->clickables);
  free(ov);
}

void grab_keyboard_and_mouse(overlay_t *ov) {
  xcb_grab_keyboard_cookie_t cookie =
      xcb_grab_keyboard(ov->conn, 1, ov->screen->root, XCB_CURRENT_TIME,
                        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);

  xcb_grab_keyboard_reply_t *reply =
      xcb_grab_keyboard_reply(ov->conn, cookie, NULL);

  if (!reply || reply->status != XCB_GRAB_STATUS_SUCCESS) {
    fprintf(stderr, "Couldn't grab keyboard\n");
  } else {
    fprintf(stderr, "Keyboard grabbed\n");
  }

  free(reply);
  uint16_t mask = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
                  XCB_EVENT_MASK_POINTER_MOTION;

  xcb_grab_pointer_cookie_t pcookie = xcb_grab_pointer(
      ov->conn, 1, ov->screen->root, mask, XCB_GRAB_MODE_ASYNC,
      XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE, XCB_CURRENT_TIME);

  xcb_grab_pointer_reply_t *preply =
      xcb_grab_pointer_reply(ov->conn, pcookie, NULL);

  if (!preply || preply->status != XCB_GRAB_STATUS_SUCCESS) {
    fprintf(stderr, "Couldn't grab pointer\n");
  } else {
    fprintf(stderr, "Mouse grabbed\n");
  }

  free(preply);
}

void overlay_run(overlay_t *ov) {
  xcb_generic_event_t *event;
  grab_keyboard_and_mouse(ov);

  while (ov->running && (event = xcb_wait_for_event(ov->conn))) {
    uint8_t type = event->response_type & ~0x80;

    switch (type) {
    case XCB_EXPOSE: {
      xcb_expose_event_t *ev = (xcb_expose_event_t *)event;
      if (ev->count == 0) {
        overlay_repaint(ov);
      }
      break;
    }
    case XCB_KEY_PRESS: {
      xcb_key_press_event_t *ev = (xcb_key_press_event_t *)event;
      if ((ev->state & XCB_MOD_MASK_CONTROL) && ev->detail == 41) { // F
        fprintf(stderr, "Resuming activity: no curious paws around...\n");
        xcb_ungrab_keyboard(ov->conn, XCB_CURRENT_TIME);
        xcb_ungrab_pointer(ov->conn, XCB_CURRENT_TIME);
        xcb_flush(ov->conn);
        ov->running = 0;
      }
      break;
    }
    case XCB_BUTTON_PRESS: {
      xcb_button_press_event_t *ev = (xcb_button_press_event_t *)event;
      overlay_handle_click(ov, ev->event_x, ev->event_y, ev->detail);
      break;
    }
    case XCB_MOTION_NOTIFY: {
      xcb_motion_notify_event_t *ev = (xcb_motion_notify_event_t *)event;
      overlay_handle_hover(ov, ev->event_x, ev->event_y);
      break;
    }
    case XCB_DESTROY_NOTIFY:
      ov->running = 0;
      break;

    default:
      // fprintf(stderr, "Response not supported code(%d)\n",
      //       event->response_type);
      break;
    }

    free(event);
  }
}
