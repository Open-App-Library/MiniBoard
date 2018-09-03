#ifndef CANVAS_H
#define CANVAS_H

#include <gtk/gtk.h>

#include "canvas.h"
#include "gui.h"

cairo_surface_t *get_source_canvas();

cairo_surface_t *get_user_canvas();

int get_source_canvas_width();
int get_source_canvas_height();

int get_user_canvas_width();
int get_user_canvas_height();

int get_canvas_x_offset();
int get_canvas_y_offset();

gdouble get_scale_value();

cairo_surface_t *new_canvas();

cairo_surface_t *new_canvas_with_dimensions(int canvas_width, int canvas_height);

void clear_specific_canvas(cairo_surface_t *canvas);

void clear_canvas ();

void destroy_canvases();

void scale_canvas(gdouble scale, gdouble x, gdouble y);
void scale_canvas_from(gdouble scale_from, gdouble scale_to, gdouble x, gdouble y);

void set_scale_value_on_release(gdouble last_scale);

gboolean source_canvas_exists();
gboolean user_canvas_exists();

gboolean allowed_to_draw();
void     set_allowed_to_draw(gboolean bool);

gboolean
draw_cb (GtkWidget *widget,
         cairo_t   *cr,
         gpointer     data);

gboolean
configure_event_cb (GtkWidget           *widget,
                    GdkEventConfigure *event,
                    gpointer           data);

#endif

