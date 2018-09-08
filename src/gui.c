#include <stdio.h>
#include <math.h>
#include "gui.h"
#include "canvas.h"
#include "brush.h"

#include <gtk/gtk.h>

static GtkBuilder *builder;
static GtkWidget  *app_window;
static GtkWidget  *drawing_frame;
static GtkWidget  *drawing_area;

static GtkWidget  *header_bar;
static GtkWidget  *brush_color_widget;
static GtkWidget  *brush_size_widget;
static GtkWidget  *actions_menu;
static GtkWidget  *new_button;
static GtkWidget  *open_button;
static GtkWidget  *save_button;
static GtkWidget  *save_as_button;
static GtkWidget  *eraser_toggle_button;
static GtkWidget  *pan_toggle_button;

static gdouble mouse_start_location_x;
static gdouble mouse_start_location_y;
static gdouble last_x_offset_value = 0.0;
static gdouble last_y_offset_value = 0.0;

static gdouble last_scale_value = 1.0;
static gdouble current_scale_value = 1.0;

// Keyboard/Mouse-related variables
static gboolean control_key_is_down = FALSE;
static gboolean left_mouse_button_is_down = FALSE;

gchar *active_filename;

GdkRGBA get_brush_color()
{
  return brush_color_value;
}
int     get_brush_size()
{
  return brush_size_value;
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
  header_bar           = GTK_WIDGET(gtk_builder_get_object(builder, "header_bar"));
  brush_color_widget   = GTK_WIDGET(gtk_builder_get_object(builder, "brush_color"));
  brush_size_widget    = GTK_WIDGET(gtk_builder_get_object(builder, "brush_size"));
  actions_menu         = GTK_WIDGET(gtk_builder_get_object(builder, "actions_menu"));
  new_button           = GTK_WIDGET(gtk_builder_get_object(builder, "new_button"));
  open_button          = GTK_WIDGET(gtk_builder_get_object(builder, "open_button"));
  save_button          = GTK_WIDGET(gtk_builder_get_object(builder, "save_button"));
  save_as_button       = GTK_WIDGET(gtk_builder_get_object(builder, "save_as_button"));
  eraser_toggle_button = GTK_WIDGET(gtk_builder_get_object(builder, "eraser_toggle_button"));
  pan_toggle_button    = GTK_WIDGET(gtk_builder_get_object(builder, "pan_toggle_button"));

  drawing_area = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER(drawing_frame), drawing_area);

  /* Signals used to handle the backing surface */
  g_signal_connect (drawing_area, "draw",
                    G_CALLBACK (draw_cb), NULL);
  g_signal_connect (drawing_area,"configure-event",
                    G_CALLBACK (configure_event_cb), NULL);

  /* Event signals */
  g_signal_connect (app_window, "key-press-event",
                    G_CALLBACK (key_press_cb), NULL);
  g_signal_connect (app_window, "key-release-event",
                    G_CALLBACK (key_release_cb), NULL);

  g_signal_connect (drawing_area, "motion-notify-event",
                    G_CALLBACK (motion_notify_event_cb), NULL);
  g_signal_connect (drawing_area, "scroll-event",
                    G_CALLBACK (scroll_event_cb), NULL);
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

  g_signal_connect(new_button, "clicked",
                   G_CALLBACK(new_button_clicked), NULL);
  g_signal_connect(open_button, "clicked",
                   G_CALLBACK(open_button_clicked), NULL);
  g_signal_connect(save_as_button, "clicked",
                   G_CALLBACK(save_as_button_clicked), NULL);
  g_signal_connect(save_button, "clicked",
                   G_CALLBACK(save_button_clicked), NULL);

  // Pinch-to-zoom
  GtkGesture *pinch = gtk_gesture_zoom_new(drawing_area);
  gtk_event_controller_set_propagation_phase  (GTK_EVENT_CONTROLLER(pinch),
                                              GTK_PHASE_TARGET);
  g_signal_connect(pinch, "scale-changed",
                   G_CALLBACK(gesture_zoom_event), NULL);
  g_signal_connect(pinch, "begin",
                   G_CALLBACK(gesture_begin), NULL);
  g_signal_connect(pinch, "end",
                   G_CALLBACK(gesture_end), NULL);

  /* Ask to receive events the drawing area doesn't normally
   * subscribe to. In particular, we need to ask for the
   * button press and motion notify events that want to handle.
   */
  gtk_widget_set_events (drawing_area, gtk_widget_get_events(drawing_area)
                                     | GDK_BUTTON_PRESS_MASK
                                     | GDK_BUTTON_RELEASE_MASK
                                     | GDK_POINTER_MOTION_MASK
                                     | GDK_TOUCH_MASK
                                     | GDK_SCROLL_MASK);


  gtk_widget_show_all(GTK_WIDGET(app_window));

  gtk_main ();

  return 0;
}

gboolean save_as_button_clicked (GtkWidget      *widget,
                                 GdkEventButton *event,
                                 gpointer        data)
{
  GtkFileChooserNative *native;
  GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
  gint res;

  native = gtk_file_chooser_native_new ("Save File",
                                        GTK_WINDOW(app_window),
                                        action,
                                        "_Save",
                                        "_Cancel");

  GtkFileFilter *pngfilter = gtk_file_filter_new();
  gtk_file_filter_set_name(pngfilter, "PNG Image");
  gtk_file_filter_add_pattern(pngfilter, "*.png");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(native), pngfilter);

  res = gtk_native_dialog_run (GTK_NATIVE_DIALOG (native));
  if (res == GTK_RESPONSE_ACCEPT)
    {
      char *filename;
      GtkFileChooser *chooser = GTK_FILE_CHOOSER (native);
      filename = gtk_file_chooser_get_filename (chooser);
      filename = g_strconcat(filename, ".png", NULL);
      active_filename = g_strconcat(filename, NULL); // essentially copies the point to active_filename
      save_button_clicked(widget, event, data);

      g_free (filename);
    }

  g_object_unref (native);

  return TRUE;
}

gboolean save_button_clicked (GtkWidget      *widget,
                              GdkEventButton *event,
                              gpointer        data)
{
  if (!active_filename) {
    save_as_button_clicked(widget, event, data);
  } else {
    cairo_surface_write_to_png(get_source_canvas(), active_filename);
    gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header_bar), active_filename);
    printf("File saved to %s\n", active_filename);
  }
  // If actions menu is open, close it
  gtk_popover_popdown(GTK_POPOVER(actions_menu));
  return TRUE;
}


gboolean open_button_clicked (GtkWidget      *widget,
                              GdkEventButton *event,
                              gpointer        data)
{
  GtkFileChooserNative *native;
  GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
  gint res;

  native = gtk_file_chooser_native_new ("Open File",
                                        GTK_WINDOW(app_window),
                                        action,
                                        "_Open",
                                        "_Cancel");

  GtkFileFilter *pngfilter = gtk_file_filter_new();
  gtk_file_filter_set_name(pngfilter, "PNG Image");
  gtk_file_filter_add_pattern(pngfilter, "*.png");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(native), pngfilter);

  res = gtk_native_dialog_run (GTK_NATIVE_DIALOG (native));
  if (res == GTK_RESPONSE_ACCEPT)
    {
      char *filename;
      GtkFileChooser *chooser = GTK_FILE_CHOOSER (native);
      filename = gtk_file_chooser_get_filename (chooser);
      active_filename = g_strconcat(filename, NULL); // essentially copies the point to active_filename
      set_canvas_from_png(filename);
      gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header_bar), active_filename);
      printf("Loaded %s\n", active_filename);
      // If actions menu is open, close it
      gtk_popover_popdown(GTK_POPOVER(actions_menu));

      g_free (filename);
    }

  g_object_unref (native);

  return TRUE;
}

gboolean new_button_clicked(GtkWidget      *widget,
                            GdkEventButton *event,
                            gpointer        data)
{
  reset_canvas();
  active_filename = NULL;
  gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header_bar), "Untitled Drawing");
  gtk_popover_popdown(GTK_POPOVER(actions_menu));

  return TRUE;
}

gboolean button_press_event_cb (GtkWidget      *widget,
                                GdkEventButton *event,
                                gpointer        data)
{
  // paranoia check, in case we haven't gotten a configure event
  if (get_source_canvas() == NULL)
    return FALSE;

  // If actions menu is open, close it
  gtk_popover_popdown(GTK_POPOVER(actions_menu));

  if (event->button == GDK_BUTTON_PRIMARY) {
    left_mouse_button_is_down = TRUE;
    mouse_start_location_x = event->x;
    mouse_start_location_y = event->y;
    last_x_offset_value = get_canvas_x_offset();
    last_y_offset_value = get_canvas_y_offset();
    if (!pan_mode_enabled()) {
      draw_brush(widget, event->x, event->y);
    }
  } else if (event->button == GDK_BUTTON_SECONDARY) {
    /* clear_canvas(); */
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
  if (event->button == GDK_BUTTON_PRIMARY) {
    left_mouse_button_is_down = FALSE;
  }
  return TRUE;
}

gboolean
motion_notify_event_cb (GtkWidget        *widget,
                        GdkEventMotion *event,
                        gpointer        data)
{
  /* paranoia check, in case we haven't gotten a configure event */
  if (get_source_canvas() == NULL)
    return FALSE;

  if (event->state & GDK_BUTTON1_MASK) {
    if (pan_mode_enabled()) {
      set_canvas_offset(last_x_offset_value + (event->x - mouse_start_location_x),
                        last_y_offset_value + (event->y - mouse_start_location_y));
    } else {
      draw_brush (widget, event->x, event->y);
    }
  }

  /* We've handled it, stop processing */
  return TRUE;
}

gboolean scroll_event_cb (GtkWidget *widget,
                          GdkEventScroll  *event,
                          gpointer   user_data)
{
  gdouble scroll_speed = 10.0 * get_scale_value();
  gdouble scale_amt    = 0.01;
  gdouble dx = 0.0, dy = 0.0, ds=0.0; // ds means delta scale
  switch (event->direction) {
    case GDK_SCROLL_UP:
      dy = scroll_speed;
      ds = scale_amt;
      break;
    case GDK_SCROLL_DOWN:
      dy = -scroll_speed;
      ds = -scale_amt;
      break;
    case GDK_SCROLL_LEFT:
      dx = -scroll_speed;
      break;
    case GDK_SCROLL_RIGHT:
      dx = scroll_speed;
      break;
    default:
      break;
  }
  // Zoom if the control key is held down. Otherwise pan the canvas.
  if (control_key_is_down) {
    gdouble newscale = get_scale_value() + ds;
    set_canvas_scale(newscale);
  } else {
    add_to_canvas_offset(dx, dy);
  }
  return TRUE;
}

gboolean gesture_zoom_event (GtkGestureZoom *gesture,
                             gdouble         scale,
                             gpointer        user_data)
{
  gdouble x,y;
  gtk_gesture_get_bounding_box_center(GTK_GESTURE(gesture), &x, &y);

  scale_canvas_from(last_scale_value, scale, x, y);
  current_scale_value = scale;

  return TRUE;
}

gboolean key_press_cb (GtkWidget *widget,
                       GdkEventKey  *event,
                       gpointer   user_data)
{
  switch (event->keyval) {
  case 65507: // keycode for control key
    control_key_is_down = TRUE;
  default:
    break;
  }
  return TRUE;
}

gboolean key_release_cb (GtkWidget *widget,
                         GdkEventKey  *event,
                         gpointer   user_data)
{
  switch (event->keyval) {
  case 65507: // keycode for control key
    control_key_is_down = FALSE;
  default:
    break;
  }
  return TRUE;
}

void
gesture_begin (GtkGesture       *gesture,
               GdkEventSequence *sequence,
               gpointer          user_data)
{
  gdouble x,y;
  gtk_gesture_get_bounding_box_center(gesture, &x, &y);
  set_x_start(x);
  set_y_start(y);

  set_allowed_to_draw(FALSE);
}

void
gesture_end (GtkGesture       *gesture,
             GdkEventSequence *sequence,
             gpointer          user_data)
{
  last_scale_value *= current_scale_value;

  set_x_stop();
  set_y_stop();
  set_allowed_to_draw(TRUE);
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
  gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(widget), &chosen_color);
  set_brush_color(chosen_color);
  return TRUE;
}

gboolean eraser_mode_enabled()
{
  return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(eraser_toggle_button));
}

gboolean pan_mode_enabled()
{
  return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pan_toggle_button));
}

void close_window ()
{
  if (get_source_canvas() || get_user_canvas())
    destroy_canvases();

  gtk_main_quit();
}
