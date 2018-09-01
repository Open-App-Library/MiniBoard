#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>

static GtkBuilder *builder;
static GObject    *app_window;
static GtkWidget  *drawing_frame;
static GtkWidget  *drawing_area;
static GtkWidget  *button_new_widget; // For creating new drawings
static GtkWidget  *brush_color_widget;
static GtkWidget  *brush_size_widget;

GdkRGBA get_brush_color();
int     get_brush_size();

int get_canvas_width();
int get_canvas_height();

void init_gui(int argc, char **argv);

gboolean button_press_event_cb (GtkWidget        *widget,
                                GdkEventButton   *event,
                                gpointer          data)

gboolean button_release_event_cb (GtkWidget      *widget,
                                  GdkEventButton *event,
                                  gpointer        data);

gboolean motion_notify_event_cb (GtkWidget      *widget,
                                 GdkEventMotion *event,
                                 gpointer        data);

gboolean brush_size_changed (GtkWidget      *widget,
                             GdkEventButton *event,
                             gpointer        data);

gboolean brush_color_changed (GtkWidget      *widget,
                              GdkEventButton *event,
                              gpointer        data);

void close_window ();

#endif

