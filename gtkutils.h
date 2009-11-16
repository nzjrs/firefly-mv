/*
 * Gtk+ utility functions for working with pointgrey dc1394 cameras
 *
 * Written by John Stowers <john.stowers@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef _GTK_UTILS_H_
#define _GTK_UTILS_H_

#include <gtk/gtk.h>
#include <dc1394/dc1394.h>

#include "utils.h"

G_BEGIN_DECLS

dc1394error_t
render_frame_to_widget(dc1394video_frame_t *frame, GtkWidget *widget, show_mode_t show);

dc1394error_t
render_frame_to_pixbuf(dc1394video_frame_t *frame, GdkPixbuf **pbdest, show_mode_t show);

G_END_DECLS

#endif
