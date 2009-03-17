/*
 * Grab an image using libdc1394
 *
 * Written by Damien Douxchamps <ddouxchamps@users.sf.net>
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
 * Description:
 *
 *  An extension to the original grab image sample program.  This demonstrates
 *  how to collect more detailed information on the various modes of the
 *  camera, convert one image format to another, and waiting so that the
 *  camera has time to capture the image before trying to write it out.
 */


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>

#include <gtk/gtk.h>
#include <dc1394/dc1394.h>

#include "config.h"
#include "utils.h"

#define MY_VIDEO_MODE DC1394_VIDEO_MODE_FORMAT7_0
#define MY_COLOR_CODING DC1394_COLOR_CODING_RAW8
#define MY_COLOR_FILTER DC1394_COLOR_FILTER_RGGB
#define MY_BAYER_METHOD DC1394_BAYER_METHOD_NEAREST

static gboolean delete_event( GtkWidget *widget, GdkEvent *event, gpointer data )
{
    gtk_main_quit();
    return TRUE;
}

static gboolean expose_event_callback (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    dc1394video_frame_t *frame = (dc1394video_frame_t *)data;

    if (frame && frame->image) {
        dc1394error_t err;
        //debayer raw data into rgb
        dc1394video_frame_t dest;

        dest.image = (unsigned char *)malloc(frame->size[0]*frame->size[1]*3*sizeof(unsigned char));
        dest.camera = frame->camera;
        err=dc1394_debayer_frames(frame, &dest, MY_BAYER_METHOD); 
        DC1394_ERR_RTN(err,"Could not debayer frames");

        //debayer frames does not adjust destintion values for stride, etc
        dest.size[0] = frame->size[0];
        dest.size[1] = frame->size[1];
        dest.stride = dest.size[0] * 3;

        gdk_draw_rgb_image(
                widget->window,
                widget->style->fg_gc[GTK_STATE_NORMAL],
                0, 0, 
                dest.size[0], dest.size[1], 
                GDK_RGB_DITHER_NONE, 
                dest.image, 
                dest.stride);
    }
    return TRUE;
}

int main(int argc, char *argv[])
{
    dc1394_t * d;
    dc1394camera_t *camera;
    dc1394video_frame_t *frame;
    dc1394error_t err;
    unsigned int width, height;
    GtkWidget *window, *canvas;

    d = dc1394_new ();
    if (!d)
        return 1;

    camera = dc1394_camera_new (d, MY_CAMERA_GUID);
    if (!camera) 
        return 2;

    gtk_init( &argc, &argv );

    // setup capture
    dc1394_get_image_size_from_video_mode(camera, MY_VIDEO_MODE, &width, &height);
    err=setup_color_capture(camera, MY_VIDEO_MODE, MY_COLOR_CODING);
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera),"Could not setup camera");

    // have the camera start sending us data
    err=dc1394_video_set_transmission(camera, DC1394_ON);
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera),"Could not start camera iso transmission");

    // capture one frame
    err=dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame);
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera),"Could not capture a frame");

    // stop data transmission
    err=dc1394_video_set_transmission(camera,DC1394_OFF);
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera),"Could not stop the camera");

    // create window
    gtk_widget_set_default_colormap (gdk_rgb_get_cmap());
    gtk_widget_set_default_visual (gdk_rgb_get_visual());
    window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    g_signal_connect( 
            G_OBJECT(window), "delete_event", 
            G_CALLBACK(delete_event), NULL );
    gtk_container_set_border_width( GTK_CONTAINER(window), 10 );

    canvas = gtk_drawing_area_new();
    gtk_widget_set_size_request(canvas, width, height);
    g_signal_connect (G_OBJECT (canvas), "expose_event",  
            G_CALLBACK (expose_event_callback), frame);

    gtk_container_add( GTK_CONTAINER(window), canvas );

    // go
    gtk_widget_show_all( window );
    gtk_main();

    // close camera
    cleanup_and_exit(camera);
    dc1394_free (d);

    return 0;
}
