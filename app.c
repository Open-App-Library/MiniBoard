#include <stdio.h>
#include <gtk/gtk.h>

GtkBuilder *builder;
GObject    *app_window;
GtkWidget  *drawing_frame;
GtkWidget  *drawing_area;
GtkWidget  *button_new_widget; // For creating new drawings
GtkWidget  *brush_color_widget;
GtkWidget  *brush_size_widget;

GdkRGBA brush_color_value = {0,0,0,0};
int brush_size_value = 5;

int last_draw_x = -1;
int last_draw_y = -1;

/* Surface to store current scribbles */
static cairo_surface_t *surface = NULL;
static cairo_surface_t *surface_copy = NULL;

static void
clear_surface (void)
{
  cairo_t *cr;

  cr = cairo_create (surface);

  cairo_set_source_rgb (cr, 1, 1, 1);
  cairo_paint (cr);

  cairo_destroy (cr);
}

/* Create a new surface of the appropriate size to store our scribbles */
static gboolean
configure_event_cb (GtkWidget           *widget,
                    GdkEventConfigure *event,
                    gpointer           data)
{
  static cairo_surface_t *new_surface = NULL;
  new_surface = gdk_window_create_similar_surface(gtk_widget_get_window (widget),
                                              CAIRO_CONTENT_COLOR,
                                              gtk_widget_get_allocated_width (widget),
                                              gtk_widget_get_allocated_height (widget));
  cairo_t *cr;
  // DRAW WHITE BACKGROUND
  cr = cairo_create (new_surface);
  cairo_set_source_rgb (cr, 1, 1, 1);
  cairo_paint (cr);
  cairo_destroy (cr);

  // TRANSFER OLD DATA TO NEW SURFACE
  cr = cairo_create (new_surface);
  cairo_set_source_surface(cr, surface, 0, 0);
  cairo_paint(cr);
  cairo_destroy (cr);

  cairo_surface_destroy(surface);
  surface = new_surface;

  /* We've handled the configure event, no need for further processing. */
  return TRUE;
}

/* Redraw the screen from the surface. Note that the ::draw
 * signal receives a ready-to-be-used cairo_t that is already
 * clipped to only draw the exposed areas of the widget
 */
static gboolean
draw_cb (GtkWidget *widget,
         cairo_t   *cr,
         gpointer     data)
{
  cairo_set_source_surface (cr, surface, 0, 0);
  cairo_paint (cr);

  return FALSE;
}

/* Draw a rectangle on the surface at the given position */
static void
draw_brush (GtkWidget *widget,
            gdouble      x,
            gdouble      y)
{
  cairo_t *cr;

  /* Paint to the surface, where we store our state */
  cr = cairo_create (surface);

  cairo_set_source_rgb(cr, brush_color_value.red, brush_color_value.green, brush_color_value.blue);
  cairo_set_line_width(cr, brush_size_value);

  if (last_draw_x == -1 || last_draw_y == -1) {
    cairo_move_to(cr, x, y);
  } else {
    cairo_move_to(cr, last_draw_x, last_draw_y);
  }

  last_draw_x = x;
  last_draw_y = y;

  cairo_line_to(cr, x, y);
  cairo_stroke(cr);

  cairo_destroy (cr);

  /* Now invalidate the affected region of the drawing area. */
  gtk_widget_queue_draw_area (widget, 0, 0, 1000, 1000);

}

/* Handle button press events by either drawing a rectangle
 * or clearing the surface, depending on which button was pressed.
 * The ::button-press signal handler receives a GdkEventButton
 * struct which contains this information.
 */
static gboolean
button_press_event_cb (GtkWidget      *widget,
                       GdkEventButton *event,
                       gpointer          data)
{
  /* paranoia check, in case we haven't gotten a configure event */
  if (surface == NULL)
    return FALSE;

  if (event->button == GDK_BUTTON_PRIMARY)
    {
      draw_brush (widget, event->x, event->y);
    }
  else if (event->button == GDK_BUTTON_SECONDARY)
    {
      clear_surface ();
      gtk_widget_queue_draw (widget);
    }

  /* We've handled the event, stop processing */
  return TRUE;
}

static gboolean
button_release_event_cb (GtkWidget      *widget,
                         GdkEventButton *event,
                         gpointer          data)
{
  last_draw_x = -1;
  last_draw_y = -1;
  return TRUE;
}

static gboolean brush_size_changed (
  GtkWidget      *widget,
  GdkEventButton *event,
  gpointer data
) {
  brush_size_value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
  return TRUE;
}

static gboolean brush_color_changed (
  GtkWidget      *widget,
  GdkEventButton *event,
  gpointer data
) {
  gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(widget), &brush_color_value);
  return TRUE;
}

/* Handle motion events by continuing to draw if button 1 is
 * still held down. The ::motion-notify signal handler receives
 * a GdkEventMotion struct which contains this information.
 */
static gboolean
motion_notify_event_cb (GtkWidget        *widget,
                        GdkEventMotion *event,
                        gpointer        data)
{
  /* paranoia check, in case we haven't gotten a configure event */
  if (surface == NULL)
    return FALSE;

  if (event->state & GDK_BUTTON1_MASK)
    draw_brush (widget, event->x, event->y);

  /* We've handled it, stop processing */
  return TRUE;
}

static void
close_window (void)
{
  if (surface)
    cairo_surface_destroy (surface);

  gtk_main_quit ();
}

int main (int argc, char **argv) {

  gtk_init(&argc, &argv);

  builder = gtk_builder_new_from_file("design.glade");

  app_window = gtk_builder_get_object(builder, "app_window");
  g_signal_connect (app_window, "destroy", G_CALLBACK (close_window), NULL);

  drawing_frame = GTK_WIDGET(gtk_builder_get_object(builder, "drawing_frame"));
  button_new_widget    = GTK_WIDGET(gtk_builder_get_object(builder, "button_new"));
  brush_color_widget   = GTK_WIDGET(gtk_builder_get_object(builder, "brush_color"));
  brush_size_widget    = GTK_WIDGET(gtk_builder_get_object(builder, "brush_size"));

  drawing_area = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER(drawing_frame), drawing_area);

  /* Signals used to handle the backing surface */
  g_signal_connect (drawing_area, "draw",
                    G_CALLBACK (draw_cb), NULL);
  g_signal_connect (drawing_area,"configure-event",
                    G_CALLBACK (configure_event_cb), NULL);

  /* Event signals */
  g_signal_connect (drawing_area, "motion-notify-event",
                    G_CALLBACK (motion_notify_event_cb), NULL);
  g_signal_connect (drawing_area, "button-press-event",
                    G_CALLBACK (button_press_event_cb), NULL);

  g_signal_connect (drawing_area, "button-release-event",
                    G_CALLBACK (button_release_event_cb), NULL);

  // UI Signals
  g_signal_connect(brush_size_widget, "changed",
                   G_CALLBACK(brush_size_changed), NULL);
  g_signal_connect(brush_color_widget, "color-set",
                   G_CALLBACK(brush_color_changed), NULL);

  /* Ask to receive events the drawing area doesn't normally
   * subscribe to. In particular, we need to ask for the
   * button press and motion notify events that want to handle.
p   */
  gtk_widget_set_events (drawing_area, gtk_widget_get_events(drawing_area)
                                     | GDK_BUTTON_PRESS_MASK
                                     | GDK_BUTTON_RELEASE_MASK
                                     | GDK_POINTER_MOTION_MASK);


  gtk_widget_show_all(GTK_WIDGET(app_window));

  gtk_main ();

  return 0;
}
