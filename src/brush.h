#pragma once

#include <gtk/gtk.h>

static GdkRGBA brush_color_value = {0,0,0,0};
static int brush_size_value = 10;

int last_draw_x;
int last_draw_y;

void set_brush_size(int brush_size);
void set_brush_color(GdkRGBA brush_color);

void draw_brush (GtkWidget *widget,
                 gdouble      x,
                 gdouble      y);

/* Sets last_draw_{x,y} to -1
 * this is used when the mouse is released */
void stroke_release();

