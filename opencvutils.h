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

#ifndef _OPENCV_UTILS_H_
#define _OPENCV_UTILS_H_

#include <cv.h>
#include <dc1394/dc1394.h>
#include <glib.h>

#include "utils.h"

G_BEGIN_DECLS

IplImage *dc1394_frame_get_iplimage(dc1394video_frame_t *frame);
IplImage *dc1394_capture_get_iplimage(dc1394camera_t *camera);

G_END_DECLS

#endif
