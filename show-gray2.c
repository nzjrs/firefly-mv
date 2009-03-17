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

#define MY_VIDEO_MODE DC1394_VIDEO_MODE_640x480_MONO8

static gboolean delete_event( GtkWidget *widget, GdkEvent *event, gpointer data )
{
    gtk_main_quit();
    return TRUE;
}

static gboolean expose_event_callback (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    dc1394video_frame_t *frame = (dc1394video_frame_t *)data;
    if (frame && frame->image) {
        gdk_draw_gray_image(
                widget->window,
                widget->style->fg_gc[GTK_STATE_NORMAL],
                0, 0, 
                frame->size[0] /*width*/ , frame->size[1] /*height*/, 
                GDK_RGB_DITHER_NONE, 
                frame->image, 
                frame->stride);
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
    err=dc1394_video_set_iso_speed(camera, MY_CAMERA_ISO_SPEED);
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera),"Could not set iso speed");

    err=dc1394_video_set_mode(camera, MY_VIDEO_MODE);
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera),"Could not set video mode");

    err=dc1394_video_set_framerate(camera, MY_CAMERA_FRAMERATE);
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera),"Could not set framerate");

    err=dc1394_capture_setup(camera, 4, DC1394_CAPTURE_FLAGS_DEFAULT);
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera),"Could not setup camera-\nmake sure that the video mode and framerate are\nsupported by your camera");

    dc1394_get_image_size_from_video_mode(camera, MY_VIDEO_MODE, &width, &height);

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
