#define CANVAS_WIDTH 1080
#define CANVAS_HEIGHT 720

#include "canvas.h"
#include "gui.h"

#include <gtk/gtk.h>

/* The source canvas is unaltered and the complete drawing the user is working on */
cairo_surface_t *source_canvas = NULL;
/* The user canvas is what the user sees on their screen. When they edit, they are
 * editing both the source canvas and the user canvas. The user canvas will also
 * be scaled and altered.*/
cairo_surface_t *user_canvas = NULL;

static gdouble scale_value = 1.0; // Current zoom/scale of the canvas

int     x_offset     = 0;
int     y_offset     = 0;

gboolean forbid_drawing = FALSE;

cairo_surface_t *get_source_canvas()
{
  return source_canvas;
}

cairo_surface_t *get_user_canvas()
{
  return user_canvas;
}

int get_source_canvas_width()
{
  // TODO: Implement this function
  return 1000;
}

int get_source_canvas_height()
{
  // TODO: Implement this function
  return 1000;
}

int get_user_canvas_width()
{
  return gtk_widget_get_allocated_width(get_canvas_widget());
}

int get_user_canvas_height()
{
  return gtk_widget_get_allocated_height(get_canvas_widget());
}

int get_canvas_x_offset()
{
  return x_offset;
}

int get_canvas_y_offset()
{
  return y_offset;
}

gdouble get_scale_value() {
  return scale_value;
}

/* Creates new canvas with height and width of the drawing area */
cairo_surface_t *new_canvas()
{
  cairo_surface_t *new_surface;

  new_surface = gdk_window_create_similar_surface(gtk_widget_get_window(get_canvas_widget()),
                                                  CAIRO_CONTENT_COLOR,
                                                  get_user_canvas_width(),
                                                  get_user_canvas_height());
  return new_surface;
}

/* Creates new canvas with custom width and height dimensions */
cairo_surface_t *new_canvas_with_dimensions(int canvas_width, int canvas_height)
{
  cairo_surface_t *new_surface;

  new_surface = gdk_window_create_similar_surface(gtk_widget_get_window(get_canvas_widget()),
                                                  CAIRO_CONTENT_COLOR,
                                                  canvas_width,
                                                  canvas_height);
  return new_surface;
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
  clear_specific_canvas(source_canvas);
  clear_specific_canvas(user_canvas);
}

void destroy_canvases()
{
  cairo_surface_destroy(source_canvas);
  cairo_surface_destroy(user_canvas);
}

void scale_canvas(gdouble scale, gdouble x, gdouble y)
{
  scale_value = scale;
  /* x_offset = x - CANVAS_WIDTH * scale * (x / get_user_canvas_width()); */
  /* y_offset = y - CANVAS_HEIGHT * scale * (y / get_user_canvas_height()); */

  gtk_widget_queue_draw(get_canvas_widget());
}

void scale_canvas_from(gdouble scale_from, gdouble scale_to, gdouble x, gdouble y)
{
  scale_value = scale_from * scale_to;

  int anchor_point_x = (CANVAS_WIDTH * scale_value); // Scale from center
  int anchor_point_y = (CANVAS_HEIGHT * scale_value);

  /* x_offset = get_user_canvas_width()  / 2 - anchor_point_x + x * scale_value; */
  /* y_offset = get_user_canvas_height() / 2 - anchor_point_y + y * scale_value; */

  x_offset = x - CANVAS_WIDTH * scale_value / 2; // zoom from center
  y_offset = y - CANVAS_HEIGHT * scale_value / 2;

  gtk_widget_queue_draw_area (get_canvas_widget(), 0, 0, get_user_canvas_width(), get_user_canvas_height());
}

gboolean source_canvas_exists()
{
  return source_canvas ? TRUE : FALSE;
}

gboolean user_canvas_exists()
{
  return user_canvas ? TRUE : FALSE;
}

gboolean allowed_to_draw()
{
  return !forbid_drawing;
}

void set_allowed_to_draw(gboolean bool)
{
  forbid_drawing = !bool;
}

gboolean
draw_cb (GtkWidget *widget,
         cairo_t   *cr,
         gpointer     data)
{
  cairo_translate(cr, x_offset, y_offset);
  cairo_scale(cr, scale_value, scale_value);
  cairo_set_source_surface(cr, source_canvas, 0, 0);
  cairo_paint (cr);

  return FALSE;
}

gboolean
configure_event_cb (GtkWidget         *widget,
                    GdkEventConfigure *event,
                    gpointer           data)
{
  static cairo_surface_t *tmp_canvas;
  tmp_canvas = gdk_window_create_similar_surface(gtk_widget_get_window (widget),
                                                 CAIRO_CONTENT_COLOR,
                                                 CANVAS_WIDTH,
                                                 CANVAS_HEIGHT);
  cairo_t *cr;

  // DRAW WHITE BACKGROUND
  cr = cairo_create (tmp_canvas);
  cairo_set_source_rgb (cr, 1, 1, 1);
  cairo_paint (cr);
  cairo_destroy (cr);

  // TRANSFER OLD DATA TO NEW CANVAS
  cr = cairo_create (tmp_canvas);
  cairo_set_source_surface(cr, source_canvas, 0, 0);
  cairo_paint(cr);
  cairo_destroy (cr);


  cairo_surface_destroy(source_canvas);
  source_canvas = tmp_canvas;

  /* We've handled the configure event, no need for further processing. */
  return TRUE;
}

