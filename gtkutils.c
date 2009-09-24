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

#include <stdlib.h>

#include "gtkutils.h"

dc1394error_t 
render_frame_to_widget(dc1394video_frame_t *frame, GtkWidget *widget, show_mode_t show)
{
    if (frame && frame->image && widget && widget->window && widget->style) {
        //debayer raw data into rgb
        dc1394error_t err;
        dc1394video_frame_t dest;

        switch (show) {
            case GRAY:
                gdk_draw_gray_image(
                        widget->window,
                        widget->style->fg_gc[GTK_STATE_NORMAL],
                        0, 0, 
                        frame->size[0] /*width*/ , frame->size[1] /*height*/, 
                        GDK_RGB_DITHER_NONE, 
                        frame->image, 
                        frame->stride);
                break;
            case COLOR:
                dest.image = (unsigned char *)malloc(frame->size[0]*frame->size[1]*3*sizeof(unsigned char));
                dest.color_coding = DC1394_COLOR_CODING_RGB8;

                err=dc1394_convert_frames(frame, &dest); 
                DC1394_ERR_RTN(err,"Could not convert frames");

                gdk_draw_rgb_image(
                        widget->window,
                        widget->style->fg_gc[GTK_STATE_NORMAL],
                        0, 0, 
                        frame->size[0], frame->size[1], 
                        GDK_RGB_DITHER_NONE, 
                        dest.image, 
                        frame->size[0] * 3);

                free(dest.image);
                break;

            case FORMAT7:
                dest.image = (unsigned char *)malloc(frame->size[0]*frame->size[1]*3*sizeof(unsigned char));

                err=dc1394_debayer_frames(frame, &dest, DC1394_BAYER_METHOD_NEAREST); 
                DC1394_ERR_RTN(err,"Could not debayer frames");

                gdk_draw_rgb_image(
                        widget->window,
                        widget->style->fg_gc[GTK_STATE_NORMAL],
                        0, 0, 
                        frame->size[0], frame->size[1], 
                        GDK_RGB_DITHER_NONE, 
                        dest.image, 
                        frame->size[0] * 3);

                free(dest.image);
                break;
        }
    }
    return DC1394_SUCCESS;
}

dc1394error_t
render_frame_to_pixbuf(dc1394video_frame_t *frame, GdkPixbuf **pbdest, show_mode_t show)
{
    if (frame && frame->image && pbdest) {
        dc1394error_t err;
        dc1394video_frame_t dest;

        dest.image = (unsigned char *)malloc(frame->size[0]*frame->size[1]*3*sizeof(unsigned char));
        dest.color_coding = DC1394_COLOR_CODING_RGB8;

        switch (show) {
            case GRAY:
            case COLOR:
                err=dc1394_convert_frames(frame, &dest); 
                DC1394_ERR_RTN(err,"Could not convert frames");
                break;
            case FORMAT7:
                err=dc1394_debayer_frames(frame, &dest, DC1394_BAYER_METHOD_NEAREST); 
                DC1394_ERR_RTN(err,"Could not debayer frames");
                break;
        }

        *pbdest = gdk_pixbuf_new_from_data(
                    dest.image,
                    GDK_COLORSPACE_RGB,
                    FALSE,              /* has alpha */
                    8,                  /* bpp */
                    dest.size[0],       /* width */
                    dest.size[1],       /* height */
                    dest.size[0] * 3,   /* rowstride */
                    NULL,
                    NULL);
    }
    return DC1394_SUCCESS;
}


