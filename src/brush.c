#include <math.h>
#include <stdio.h>
#include <gtk/gtk.h>

#include "brush.h"
#include "canvas.h"

int last_draw_x = -1;
int last_draw_y = -1;

void set_brush_size(int brush_size) {
  brush_size_value = brush_size;
}

void set_brush_color(GdkRGBA brush_color) {
  brush_color_value = brush_color;
}

void draw_brush (GtkWidget *widget,
                 gdouble    x,
                 gdouble    y)
{
  cairo_t *cr = cairo_create(get_canvas());

  cairo_set_source_rgb(cr, brush_color_value.red, brush_color_value.green, brush_color_value.blue);

  cairo_arc(cr, x, y, brush_size_value / 2, 0, 2 * M_PI);

  // DDA Algorithm
  if (last_draw_x != -1 && last_draw_y != -1) {
    int dx = x - last_draw_x;
    int dy = y - last_draw_y;

    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

    float Xinc = dx / (float) steps;
    float Yinc = dy / (float) steps;

    float xindex = last_draw_x;
    float yindex = last_draw_y;

    for (int i = 0; i <= steps; i++) {
      cairo_arc(cr, xindex, yindex, brush_size_value / 2, 0, 2 * M_PI);
      xindex += Xinc;
      yindex += Yinc;
    }
  }

  last_draw_x = x;
  last_draw_y = y;

  //cairo_fill(cr);
  cairo_fill(cr);

  cairo_destroy (cr);
  /* Now invalidate the affected region of the drawing area. */
  gtk_widget_queue_draw_area (widget, 0, 0, 1000, 1000);

}

void stroke_release() {
  last_draw_x = -1;
  last_draw_y = -1;
}

