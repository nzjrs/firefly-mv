// file: playback-simple.c
// auth: Albert Huang <albert@csail.mit.edu>
// date: October 12, 2005
//
// displays a recording made using record-simple.c
//
// requires libdc1394 2.0+ and gtk+ 2.0
//
// gcc playback-simple.c -o playback-simple  \
//      `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0` \ 
//      -ldc1394_control

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>
#include <inttypes.h>

#include <gtk/gtk.h>

#include <dc1394/dc1394.h>

#define FRAMEWIDTH 1024
#define FRAMEHEIGHT 4608
#define BYTESPERFRAME 4718592

char filename[1024] = { 0 };
FILE *fp = NULL;
uint8_t *rgbbuf = NULL;
uint_t current_frame = 0;

int renderframe( int );

gboolean canvas_button_press( GtkWidget *widget, GdkEventButton *event,
        gpointer data )
{
    if( event->button == 1 ) {
        current_frame++;
        if( ! renderframe( current_frame ) ) current_frame--;
    } else if ( event->button == 3 ) {
        if( current_frame > 0 ) current_frame--;
        renderframe( current_frame );
    }
    g_print("frame: %d\n", current_frame);
    gtk_widget_queue_draw_area( widget, 0, 0, 
            widget->allocation.width, widget->allocation.height);
    return TRUE;
}

gboolean
expose_event_callback (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    gdk_draw_rgb_image( widget->window, widget->style->fg_gc[GTK_STATE_NORMAL],
            0, 0, widget->allocation.width, widget->allocation.height, 
            GDK_RGB_DITHER_NONE, rgbbuf, widget->allocation.width * 3 );

    return TRUE;
}

static gboolean delete_event( GtkWidget *widget, 
        GdkEvent *event, 
        gpointer data )
{
    g_print("delete event occured\n");
    gtk_main_quit();
    return TRUE;
}

static void destroy( GtkWidget *widget, 
        gpointer data )
{
    gtk_main_quit();
}

int renderframe( int i ) 
{
    if( i < 0 ) return 0;

    // seek to frame
    fseek( fp, i * BYTESPERFRAME, SEEK_SET );

    // read in one frame
    uchar_t bayer[BYTESPERFRAME];

    size_t bytesread = fread( bayer, BYTESPERFRAME, 1, fp );

    if( bytesread != 1 ) return 0;

    // invoke bayer decoding magic
    dc1394_bayer_decoding_8bit( bayer, rgbbuf, FRAMEWIDTH, FRAMEHEIGHT, 
            DC1394_COLOR_FILTER_BGGR, DC1394_BAYER_METHOD_NEAREST );

    return 1;
}

int main( int argc, char *argv[])
{
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *vbox;
    GtkWidget *canvas;

    if( argc < 2 ) {
        printf("usage: playback-simple <filename>\n");
        exit(1);
    }
    strncpy(filename, argv[1], 1024);

    fp = fopen(filename, "rb");
    if( fp == NULL ) { 
        perror("opening file");
        exit(1);
    }

    rgbbuf = (uint8_t*) malloc( FRAMEWIDTH * FRAMEHEIGHT * 3 );

    gtk_init( &argc, &argv );
    gdk_rgb_init();

    gtk_widget_set_default_colormap (gdk_rgb_get_cmap());
    gtk_widget_set_default_visual (gdk_rgb_get_visual());

    // create window
    window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    g_signal_connect( G_OBJECT(window), "delete_event", 
            G_CALLBACK(delete_event), NULL );
    g_signal_connect( G_OBJECT(window), "destroy",
            G_CALLBACK(destroy), NULL );
    gtk_container_set_border_width( GTK_CONTAINER(window), 10 );

    // add widgets
    
    // vbox (VBox)
    vbox = gtk_vbox_new(FALSE, 10);
    gtk_container_add( GTK_CONTAINER(window), vbox );

    // canvas (DrawingArea)
    canvas = gtk_drawing_area_new();
    gtk_widget_set_size_request(canvas, FRAMEWIDTH, FRAMEHEIGHT);
    g_signal_connect (G_OBJECT (canvas), "expose_event",  
            G_CALLBACK (expose_event_callback), NULL);

    gtk_widget_set_events(canvas, GDK_LEAVE_NOTIFY_MASK
            | GDK_BUTTON_PRESS_MASK
            | GDK_BUTTON_RELEASE_MASK
            | GDK_POINTER_MOTION_MASK );

    g_signal_connect( G_OBJECT(canvas), "button_press_event", 
           G_CALLBACK(canvas_button_press), NULL );

    gtk_box_pack_start(GTK_BOX(vbox), canvas, TRUE, TRUE, 0);

    // button 
    button = gtk_button_new_with_label( "Quit" );
    g_signal_connect_swapped( G_OBJECT(button), "clicked",
            G_CALLBACK(gtk_widget_destroy),
            G_OBJECT(window) );

    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, TRUE, 0);

    // display everything
    gtk_widget_show( canvas );
    gtk_widget_show( vbox );
    gtk_widget_show( button );
    gtk_widget_show( window );

    // render the first frame
    renderframe(0);
    
    // go
    gtk_main();

    fclose(fp);
    free( rgbbuf );

    return 0;
}
