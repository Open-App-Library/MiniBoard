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

gdouble get_canvas_x_offset();
gdouble get_canvas_y_offset();
void    set_canvas_x_offset(gdouble amt);
void    set_canvas_y_offset(gdouble amt);
void    add_to_canvas_x_offset(gdouble amt);
void    add_to_canvas_y_offset(gdouble amt);

gdouble get_canvas_x_anchor();
gdouble get_canvas_y_anchor();

gdouble get_scale_value();

cairo_surface_t *new_canvas();

cairo_surface_t *new_canvas_with_dimensions(int canvas_width, int canvas_height);

void clear_specific_canvas(cairo_surface_t *canvas);

void clear_canvas();
void reset_canvas(); // For creating new files

void destroy_canvases();

void set_canvas_scale(gdouble scale);
void scale_canvas_from(gdouble scale_from, gdouble scale_to, gdouble x, gdouble y);

void set_scale_value_on_release(gdouble last_scale);

gboolean source_canvas_exists();
gboolean user_canvas_exists();

void set_canvas_from_png();

gboolean allowed_to_draw();
void     set_allowed_to_draw(gboolean bool);

void set_x_start(gdouble num);
void set_y_start(gdouble num);
void set_x_stop();
void set_y_stop();

gboolean
draw_cb (GtkWidget *widget,
         cairo_t   *cr,
         gpointer     data);

gboolean
configure_event_cb (GtkWidget           *widget,
                    GdkEventConfigure *event,
                    gpointer           data);

#endif

