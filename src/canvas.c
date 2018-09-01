#include "canvas.h"
#include "gui.h"

#include <gtk/gtk.h>

cairo_surface_t *get_canvas()
{
  return canvas;
}

void clear_canvas ()
{
  cairo_t *cr = cairo_create(canvas);

  cairo_set_source_rgb (cr, 1, 1, 1); // White color
  cairo_paint (cr);

  cairo_destroy (cr);
}

void destroy_canvas()
{
  cairo_surface_destroy(canvas);
}

gboolean canvas_exists()
{
  return canvas ? true : false;
}

gboolean
draw_cb (GtkWidget *widget,
         cairo_t   *cr,
         gpointer     data)
{
  cairo_set_source_surface (cr, canvas, 0, 0);
  cairo_paint (cr);

  return FALSE;
}

gboolean
configure_event_cb (GtkWidget         *widget,
                    GdkEventConfigure *event,
                    gpointer           data)
{
  static cairo_surface_t *new_canvas;
  new_canvas = gdk_window_create_similar_surface(gtk_widget_get_window (widget),
                                                 CAIRO_CONTENT_COLOR,
                                                 get_canvas_width(),
                                                 get_canvas_height());
  cairo_t *cr;

  // DRAW WHITE BACKGROUND
  cr = cairo_create (new_canvas);
  cairo_set_source_rgb (cr, 1, 1, 1);
  cairo_paint (cr);
  cairo_destroy (cr);

  // TRANSFER OLD DATA TO NEW CANVAS
  cr = cairo_create (new_canvas);
  cairo_scale(cr, scale, scale);
  cairo_set_source_surface(cr, canvas, 0, 0);
  cairo_paint(cr);
  cairo_destroy (cr);


  cairo_surface_destroy(canvas);
  canvas = new_canvas;

  /* We've handled the configure event, no need for further processing. */
  return TRUE;
}

