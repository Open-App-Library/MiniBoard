#include "canvas.h"
#include "gui.h"

#include <gtk/gtk.h>

static cairo_surface_t *original_canvas = NULL;
static cairo_surface_t *zoomed_canvas = NULL;

cairo_surface_t *get_canvas()
{
  return zoomed_canvas;
}

void clear_specific_canvas(cairo_surface_t *canvas)
{
  cairo_t *cr = cairo_create(canvas);
  cairo_set_source_rgb (cr, 1, 1, 1); // White color
  cairo_paint (cr);
  cairo_destroy (cr);
}

void clear_canvas ()
{
  clear_specific_canvas(original_canvas);
  clear_specific_canvas(zoomed_canvas);
}

void destroy_canvas()
{
  cairo_surface_destroy(original_canvas);
  cairo_surface_destroy(zoomed_canvas);
}

void copy_canvas()

void scale_canvas(gdouble scale, gdouble x, gdouble y)
{
  cairo_surface_t *new_surface = NULL;
  int canvas_width = gtk_widget_get_allocated_width(get_canvas_widget());
  int canvas_height = gtk_widget_get_allocated_height(get_canvas_widget());

  new_surface = gdk_window_create_similar_surface(gtk_widget_get_window(get_canvas_widget()),
                                                  CAIRO_CONTENT_COLOR,
                                                  canvas_width,
                                                  canvas_height);

  cairo_t *cr = cairo_create (new_surface);

  // cairo_scale(cr, 1/last_multiplied, 1/last_multiplied);
  cairo_scale(cr, scale, scale);

  cairo_set_source_surface(cr, canvas, 0, 0);

  cairo_translate(cr, canvas_width/2, 0);

  cairo_paint(cr);
  cairo_destroy (cr);
  cairo_surface_destroy(canvas);
  canvas = new_surface;
  gtk_widget_queue_draw_area (get_canvas_widget(), 0, 0, canvas_width, canvas_height);
}

gboolean canvas_exists()
{
  return canvas ? TRUE : FALSE;
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
  cairo_set_source_surface(cr, canvas, 0, 0);
  cairo_paint(cr);
  cairo_destroy (cr);


  cairo_surface_destroy(canvas);
  canvas = new_canvas;

  /* We've handled the configure event, no need for further processing. */
  return TRUE;
}

