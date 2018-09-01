#ifndef CANVAS_H
#define CANVAS_H

#include <gtk/gtk.h>

static cairo_surface_t *canvas = NULL;

#include "canvas.h"
#include "gui.h"

cairo_surface_t *get_canvas();

void clear_canvas();

void destroy_canvas();

gboolean canvas_exists();

gboolean
draw_cb (GtkWidget *widget,
         cairo_t   *cr,
         gpointer     data);

gboolean
configure_event_cb (GtkWidget           *widget,
                    GdkEventConfigure *event,
                    gpointer           data);

#endif

