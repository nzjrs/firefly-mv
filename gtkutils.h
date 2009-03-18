#ifndef _GTK_UTILS_H_
#define _GTK_UTILS_H_

#include <gtk/gtk.h>
#include <dc1394/dc1394.h>

#include "utils.h"

dc1394error_t
render_frame_to_widget(dc1394video_frame_t *frame, GtkWidget *widget, show_mode_t show);

#endif
