/*
 * Play recorded video from the dc1394 camera
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
 * Description:
 *    Displays images recorded previously using dc1394-record. Left
 *    mouse button advances frames, right mouse button returns to
 *    the previous frame.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <dc1394/dc1394.h>

#include "camera.h"
#include "utils.h"
#include "gtkutils.h"

typedef struct __playback
{
    char                *filename;
    FILE                *fp;
    uint64_t            frame_number;
    dc1394video_frame_t frame;
    long                total_frame_size;
    show_mode_t         show;
} playback_t;

static int 
renderframe(int i, playback_t *play) 
{
    if( i < 0 )
        return 0;

    if (fseek(play->fp, i * play->total_frame_size, SEEK_SET) == 0) {
        if (play->frame.image) {
            free(play->frame.image);
            play->frame.image = NULL;
        }
        read_frame( &(play->frame), play->fp );
        return 1;
    } else {
        return 0;
    }
}

static gboolean 
canvas_button_press( GtkWidget *widget, GdkEventButton *event, gpointer data )
{
    playback_t *play = (playback_t *)data;

    if( event->button == 1 ) {
        play->frame_number++;
        if( !renderframe(play->frame_number, play) ) 
            play->frame_number--;
    } else if ( event->button == 3 ) {
        if( play->frame_number > 0 ) 
            play->frame_number--;
        renderframe( play->frame_number, play );
    }
    
    g_print("frame: %lld\n", play->frame_number);

    gtk_widget_queue_draw_area( widget, 0, 0, 
            widget->allocation.width, widget->allocation.height);
    return TRUE;
}

static gboolean
expose_event_callback (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    playback_t *play = (playback_t *)data;

    render_frame_to_widget(&(play->frame), widget, play->show);

    return TRUE;
}

static gboolean 
delete_event( GtkWidget *widget, GdkEvent *event, gpointer data )
{
    gtk_main_quit();
    return TRUE;
}

int main( int argc, char *argv[])
{
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *vbox;
    GtkWidget *canvas;

    playback_t play = { 0 };

    /* Option parsing */
    GError *error = NULL;
    GOptionContext *context;
    GOptionEntry entries[] =
    {
      { "input-filename", 'i', 0, G_OPTION_ARG_FILENAME, &(play.filename), "Input filename", "FILE" },
      { NULL }
    };

    context = g_option_context_new("- Firefly MV Camera Playback");
    g_option_context_add_main_entries (context, entries, NULL);

    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        printf( "Error: %s\n%s", 
                error->message, 
                g_option_context_get_help(context, TRUE, NULL));
        exit(1);
    }
    if (play.filename == NULL) {
        printf( "Error: You must supply a filename\n%s", 
                g_option_context_get_help(context, TRUE, NULL));
        exit(2);
    }

    if (play.filename[0] == '-') {
        play.fp = stdin;
    } else {
        play.fp = fopen(play.filename, "rb");
    }

    if( play.fp == NULL ) { 
        perror("opening file");
        exit(1);
    }

    // read the first frame
    play.total_frame_size = read_frame(&play.frame, play.fp);
    if (play.frame.color_coding == DC1394_COLOR_CODING_MONO8)
        play.show = GRAY;
    else if (play.frame.color_coding == DC1394_COLOR_CODING_RGB8)
        play.show = COLOR;
    else if (play.frame.color_coding == DC1394_COLOR_CODING_RAW8)
        play.show = FORMAT7;
    else {
        perror("invalid color coding");
        exit(1);
    }

    gtk_init( &argc, &argv );
    gdk_rgb_init();

    gtk_widget_set_default_colormap (gdk_rgb_get_cmap());
    gtk_widget_set_default_visual (gdk_rgb_get_visual());

    // create window
    window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    g_signal_connect( G_OBJECT(window), "delete_event", G_CALLBACK(delete_event), NULL );
    g_signal_connect( G_OBJECT(window), "destroy", G_CALLBACK(delete_event), NULL );
    gtk_container_set_border_width( GTK_CONTAINER(window), 10 );

    // add widgets
    // vbox (VBox)
    vbox = gtk_vbox_new(FALSE, 10);
    gtk_container_add( GTK_CONTAINER(window), vbox );

    // canvas (DrawingArea)
    canvas = gtk_drawing_area_new();
    gtk_widget_set_size_request(canvas, play.frame.size[0], play.frame.size[1]);
    g_signal_connect (G_OBJECT (canvas), "expose_event",  
            G_CALLBACK (expose_event_callback), &play);

    gtk_widget_set_events(canvas, GDK_LEAVE_NOTIFY_MASK
            | GDK_BUTTON_PRESS_MASK
            | GDK_BUTTON_RELEASE_MASK
            | GDK_POINTER_MOTION_MASK );

    g_signal_connect( G_OBJECT(canvas), "button_press_event", 
           G_CALLBACK(canvas_button_press), &play);

    gtk_box_pack_start(GTK_BOX(vbox), canvas, TRUE, TRUE, 0);

    // button 
    button = gtk_button_new_with_label( "Quit" );
    g_signal_connect_swapped( G_OBJECT(button), "clicked",
            G_CALLBACK(gtk_widget_destroy),
            G_OBJECT(window) );

    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, TRUE, 0);

    // display everything
    gtk_widget_show_all( window );

    // render the first frame
    renderframe(0, &play);
    
    // go
    gtk_main();

    fclose(play.fp);

    return 0;
}
