#include <stdio.h>
#include <math.h>
#include "gui.h"
#include "canvas.h"
#include "brush.h"

#include <gtk/gtk.h>

GdkRGBA get_brush_color()
{
  return brush_color_value;
}
int     get_brush_size()
{
  return brush_size_value;
}

int get_canvas_width()
{
  return 1000;
}

int get_canvas_height()
{
  return 1000;
}

GtkWidget *get_canvas_widget()
{
  return drawing_area;
}

GtkWidget *get_window_widget()
{
  return app_window;
}

int init_gui(int *argc, char ***argv) {
  gtk_init(argc, argv);

  builder = gtk_builder_new_from_resource("/io/dougie/miniboard/design.glade");

  app_window = GTK_WIDGET(gtk_builder_get_object(builder, "app_window"));
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
  g_signal_connect (app_window, "button-release-event",
                    G_CALLBACK (button_release_event_cb), NULL);

  g_signal_connect(brush_size_widget, "changed",
                   G_CALLBACK(brush_size_changed), NULL);
  g_signal_connect(brush_color_widget, "color-set",
                   G_CALLBACK(brush_color_changed), NULL);

  // Pinch-to-zoom
  GtkGesture *pinch = gtk_gesture_zoom_new(drawing_area);
  gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER(pinch),
                                              GTK_PHASE_TARGET);
  g_signal_connect(pinch, "scale-changed",
                   G_CALLBACK(gesture_zoom_event), NULL);

  /* Ask to receive events the drawing area doesn't normally
   * subscribe to. In particular, we need to ask for the
   * button press and motion notify events that want to handle.
   */
  gtk_widget_set_events (drawing_area, gtk_widget_get_events(drawing_area)
                                     | GDK_BUTTON_PRESS_MASK
                                     | GDK_BUTTON_RELEASE_MASK
                                     | GDK_POINTER_MOTION_MASK
                                     | GDK_TOUCH_MASK);


  gtk_widget_show_all(GTK_WIDGET(app_window));

  gtk_main ();

  return 0;
}

gboolean button_press_event_cb (GtkWidget      *widget,
                                GdkEventButton *event,
                                gpointer        data)
{
  // paranoia check, in case we haven't gotten a configure event
  if (get_canvas() == NULL)
    return FALSE;

  if (event->button == GDK_BUTTON_PRIMARY) {
    draw_brush(widget, event->x, event->y);
  } else if (event->button == GDK_BUTTON_SECONDARY) {
    clear_canvas();
    gtk_widget_queue_draw(widget);
  }

  /* We've handled the event, stop processing */
  return TRUE;
}

gboolean button_release_event_cb (GtkWidget      *widget,
                                  GdkEventButton *event,
                                  gpointer        data)
{
  stroke_release();
  return TRUE;
}


gboolean
motion_notify_event_cb (GtkWidget        *widget,
                        GdkEventMotion *event,
                        gpointer        data)
{
  /* paranoia check, in case we haven't gotten a configure event */
  if (get_canvas() == NULL)
    return FALSE;

  if (event->state & GDK_BUTTON1_MASK)
    draw_brush (widget, event->x, event->y);

  /* We've handled it, stop processing */
  return TRUE;
}

gboolean gesture_zoom_event (GtkGestureZoom *controller,
                             gdouble         scale,
                             gpointer        user_data)
{
  gdouble x,y;
  gtk_gesture_get_bounding_box_center(GTK_GESTURE(controller), &x, &y);

  scale_canvas(scale, x, y);


  return TRUE;
}

gboolean brush_size_changed (GtkWidget      *widget,
                                    GdkEventButton *event,
                                    gpointer        data)
{
  set_brush_size(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget)));
  return TRUE;
}

gboolean brush_color_changed (GtkWidget      *widget,
                                     GdkEventButton *event,
                                     gpointer        data)
{
  GdkRGBA chosen_color;
  gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(widget), &brush_color_value);
  set_brush_color(chosen_color);
  return TRUE;
}

void close_window ()
{
  if (get_canvas())
    destroy_canvas();

  gtk_main_quit();
}
