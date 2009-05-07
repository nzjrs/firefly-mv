#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/time.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <dc1394/dc1394.h>

#include "config.h"
#include "utils.h"
#include "gtkutils.h"
#include "cv.h"
#include "highgui.h"

#define FPS_TO_MS(x) ((1.0/x)*1000.0)
#define IM_SAVE_PATH /tmp/hdrvideo
#define IM_BASE_NAME frame
#define IM_CHANNELS 1

typedef struct __view
{
    show_mode_t             show;
    dc1394video_frame_t     *frame;
    dc1394camera_t          *camera;
} view_t;

static gboolean delete_event( GtkWidget *widget, GdkEvent *event, gpointer data )
{
    gtk_main_quit();
    return TRUE;
}

static gboolean expose_event_callback (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    dc1394error_t err;
    view_t *view = (view_t *)data;
    static int frameindex = 0;

    err=dc1394_capture_dequeue(view->camera, DC1394_CAPTURE_POLICY_WAIT, &(view->frame));
    DC1394_WRN(err,"Could not capture a frame");

// TODO TODO TODO    

//write_frame(view->frame, fp);
    IplImage* m_image = cvCreateImageHeader(cvSize(640,480), IPL_DEPTH_8U, IM_CHANNELS);
    m_image->imageData = (char *) view->frame->image;
    m_image->imageDataOrigin = (char *) view->frame->image;

    // New OpenCV feature, not present in our version
    //int p[3];
    //p[0] = CV_IMWRITE_JPEG_QUALITY;
    //p[1] = 100;
    //p[2] = 0;

    //cvSaveImage(strcat("/tmp/hdrvideo/frame", ".jpg"), img, p);
    

    cvSaveImage("/tmp/hdrvideo/frame.jpg", m_image);

    cvReleaseImageHeader(&m_image);

    render_frame_to_widget(view->frame, widget, view->show);
        
    err=dc1394_capture_enqueue(view->camera, view->frame);
    DC1394_WRN(err,"releasing buffer");

    return TRUE;
}

static gboolean redraw(gpointer data)
{
    gtk_widget_queue_draw(GTK_WIDGET(data));
    return TRUE;
}

int main(int argc, char **argv)
{
    FILE *fp = NULL;
    unsigned char use_stdout = 0;
    uint32_t width, height;
    dc1394_t * d;
    dc1394error_t err;
    GtkWidget *window, *canvas;
    char *format = NULL;
    view_t view;
    char *filename;
    double framerate;
    int exposure, brightness, brackets, ev, i;

    /* Option parsing */
    GError *error = NULL;
    GOptionContext *context;
    GOptionEntry entries[] =
    {
      { "format", 'f', 0, G_OPTION_ARG_STRING, &format, "Format of image", "g,c,7" },
      { "output-filename", 'o', 0, G_OPTION_ARG_FILENAME, &filename, "Output filename", "FILE" },
      { "bracketcount", 'c', 0, G_OPTION_ARG_INT, &brackets, "Number of brackets", NULL },
      { "EV", 'v', 0, G_OPTION_ARG_INT, &ev, "Exposure variation", NULL },
      GOPTION_ENTRY_CAMERA_SETUP_ARGUMENTS(&framerate, &exposure, &brightness),
      { NULL }
    };

    context = g_option_context_new("- Firefly MV Camera Recorder");
    g_option_context_add_main_entries (context, entries, NULL);

    /* Defaults */
    format = NULL;
    filename = NULL;
    brackets = 0;
    ev = 0;
    exposure = -1;
    brightness = -1;
    view.show = GRAY;
    framerate = 30.0;


    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        printf( "Error: %s\n%s",
                error->message,
                g_option_context_get_help(context, TRUE, NULL));
        exit(1);
    }
    if (filename == NULL) {
        printf( "Error: You must supply a filename\n%s",
                g_option_context_get_help(context, TRUE, NULL));
        exit(2);
    }
    if (brackets <= 0) {
        printf( "Error: You must supply the number of brackets\n%s",
                g_option_context_get_help(context, TRUE, NULL));
        exit(3);
    }
    if (ev <= 0) {
        printf( "Error: You must supply an value for the exposure variation (EV)\n%s",
                g_option_context_get_help(context, TRUE, NULL));
        exit(4);
    }

    if (format && format[0])
        view.show = format[0];

    switch (view.show) {
        case GRAY:
        case COLOR:
        case FORMAT7:
            printf("Selected mode: %c\n", view.show);
            break;
        default:
            printf("Invalid Mode\n%s", g_option_context_get_help(context, TRUE, NULL));
            exit(1);
    }

    if (filename[0] == '-') {
        use_stdout = 1;
        fp = stdout;
    } else {
        fp = fopen( filename, "wb+");
    }


    if( fp == NULL ) {
        perror("creating output file");
        exit(4);
    }

    // Calculate correct exposures and setup camera


    d = dc1394_new ();
    if (!d)
        exit(5);

    view.camera = dc1394_camera_new (d, MY_CAMERA_GUID);
    if (!view.camera) 
        exit(6);

    gtk_init( &argc, &argv );

    // setup capture
    switch (view.show) {
        case GRAY:
        case COLOR:
            dc1394_get_image_size_from_video_mode(view.camera, DC1394_VIDEO_MODE_640x480_MONO8, &width, &height);
            err=setup_gray_capture(view.camera, DC1394_VIDEO_MODE_640x480_MONO8);
            break;
        case FORMAT7:
            dc1394_get_image_size_from_video_mode(view.camera, DC1394_VIDEO_MODE_FORMAT7_0, &width, &height);
            err=setup_color_capture(view.camera, DC1394_VIDEO_MODE_FORMAT7_0, DC1394_COLOR_CODING_RAW8);
            break;
    }
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(view.camera),"Could not setup camera");

    err=setup_from_command_line(view.camera, framerate, exposure, brightness);
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(view.camera),"Could not set camera from command line arguments");


    // have the camera start sending us data
    err=dc1394_video_set_transmission(view.camera, DC1394_ON);
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(view.camera),"Could not start camera iso transmission");

    // throw away some frames to prevent corruption from some modes taking a
    // few frames to change.... dunno why
    i = 3;
    while (i--) {
        dc1394_capture_dequeue(view.camera, DC1394_CAPTURE_POLICY_WAIT, &(view.frame));
        dc1394_capture_enqueue(view.camera,view.frame);
    }

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
            G_CALLBACK (expose_event_callback), &view);

    gtk_container_add( GTK_CONTAINER(window), canvas );

    // setup refresh rate
    g_timeout_add (FPS_TO_MS(20), redraw, canvas);

    // go
    gtk_widget_show_all( window );
    gtk_main();

    // stop data transmission
    err=dc1394_video_set_transmission(view.camera,DC1394_OFF);
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(view.camera),"Could not stop the camera");


    // close camera
    cleanup_and_exit(view.camera);
    dc1394_free (d);

    fclose(fp);
	return 0;
}
