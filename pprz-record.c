#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/time.h>
#include <pthread.h>

#include <glib.h>
#include <dc1394/dc1394.h>

#include "camera.h"
#include "utils.h"
#include "serial.h"
#include "ppzutils.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv)
{
    FILE *fp = NULL;
    unsigned int serial;
    unsigned char use_stdout = 0;
    uint32_t width, height;
    dc1394_t * d;
    dc1394camera_t *camera;
    dc1394error_t err;
    dc1394video_frame_t *frame;

    pthread_t thread;
    PprzParser_t parser;

    GTimeVal time;

    /* Options */
    show_mode_t show;
    char *format;
    char *filename;
    double framerate;
    int exposure, brightness, duration, i;
    long total_frame_size;
    char *serial_port;
    int serial_baud;

    /* This structure, containing the payload and other timely information
       is written to disk with the image file */
    PprzRecord_t record;

    /* Option parsing */
    GError *error = NULL;
    GOptionContext *context;
    GOptionEntry entries[] =
    {
      { "format", 'f', 0, G_OPTION_ARG_STRING, &format, "Format of image", "g,c,7" },
      { "output-filename", 'o', 0, G_OPTION_ARG_FILENAME, &filename, "Output filename", "FILE" },
      { "duration", 'd', 0, G_OPTION_ARG_INT, &duration, "Seconds to record", NULL },
      GOPTION_ENTRY_CAMERA_SETUP_ARGUMENTS(&framerate, &exposure, &brightness),
      GOPTION_ENTRY_SERIAL_SETUP_ARGUMENTS(&serial_port, &serial_baud),
      { NULL }
    };

    context = g_option_context_new("- Record image frames and telemetry data");
    g_option_context_add_main_entries (context, entries, NULL);

    /* Defaults */
    serial_port = DEFAULT_SERIAL_PORT;
    serial_baud = DEFAULT_SERIAL_BAUD;
    format = NULL;
    filename = NULL;
    show = GRAY;
    framerate = 30.0;
    exposure = -1;
    brightness = -1;
    duration = 0;

    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        printf( "Error: %s\n%s", 
                error->message, 
                g_option_context_get_help(context, TRUE, NULL));
        exit(1);
    }
    if (filename == NULL) {
        printf( "Error: You must supply a filename\n%s", 
                g_option_context_get_help(context, TRUE, NULL));
        exit(1);
    }
    if (duration <= 0) {
        printf( "Error: You must supply a duration\n%s", 
                g_option_context_get_help(context, TRUE, NULL));
        exit(1);
    }
    if (serial_port == NULL || serial_baud < 0)
    {
        printf( "Error: You must supply a serial port and speed\n%s", 
                g_option_context_get_help(context, TRUE, NULL));
        exit(1);
    }


    if (format && format[0])
        show = format[0];

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

    d = dc1394_new ();
    if (!d)
        exit(5);

    camera = dc1394_camera_new (d, MY_CAMERA_GUID);
    if (!camera)
        exit(6);

    if (!use_stdout) {
        printf( "Recording Details:\n"
                "  Duration     = %d\n"
                "  Format       = %c\n"
                "  Framerate    = %f\n"
                "  Exposure     = %d\n"
                "  Brightness   = %d\n"
                "  Serial port  = %s\n"
                "  Serial baud  = %d\n\n"
                "Recording:\n",
                duration,show,framerate,exposure,brightness,serial_port, serial_baud);
    }

    // setup capture
    switch (show) {
        case GRAY:
        case COLOR:
            dc1394_get_image_size_from_video_mode(camera, DC1394_VIDEO_MODE_640x480_MONO8, &width, &height);
            err=setup_gray_capture(camera, DC1394_VIDEO_MODE_640x480_MONO8);
            break;
        case FORMAT7:
            dc1394_get_image_size_from_video_mode(camera, DC1394_VIDEO_MODE_FORMAT7_0, &width, &height);
            err=setup_color_capture(camera, DC1394_VIDEO_MODE_FORMAT7_0, DC1394_COLOR_CODING_RAW8);
            break;
    }
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera),"Could not setup camera");

    err=setup_from_command_line(camera, framerate, exposure, brightness);
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera),"Could not set camera from command line arguments");

    // have the camera start sending us data
    err=dc1394_video_set_transmission(camera, DC1394_ON);
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera),"Could not start camera iso transmission");

    // open the serial port and start the serial thread
    parser.serial = serial_open(serial_port, serial_baud);
    serial_set_blocking(parser.serial);
    pthread_create( &thread, NULL, parse_pppz_thread, (void*) &parser);

    // throw away some frames to prevent corruption from some modes taking a 
    // few frames to change.... dunno why
    i = 3;
    while (i--) {
        dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame);
        dc1394_capture_enqueue(camera,frame);
    }

    // compute actual framerate
    struct timeval start, now;
    gettimeofday( &start, NULL );
    now = start;
    int numframes = 0;
    unsigned long elapsed = (now.tv_usec / 1000 + now.tv_sec * 1000) - 
        (start.tv_usec / 1000 + start.tv_sec * 1000);

    while(elapsed < duration * 1000)
    {
        // get a single frame
        err=dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame);
        DC1394_WRN(err,"Could not capture a frame");

        // timestamp it
        add_timestamp_to_frame(frame);

        // Copy latest data from rx thread
        pthread_mutex_lock( &mutex );
        memcpy(record.data, parser.data, MESSAGE_LENGTH_EMAV_STATE);  
        record.timestamp = parser.timestamp;    
        pthread_mutex_unlock( &mutex );

        // Write frame and data to file
        total_frame_size = write_frame_with_extras(frame, fp, (uint8_t *)&record, PPRZ_RECORD_SIZE);
        
        err=dc1394_capture_enqueue(camera,frame);
        DC1394_WRN(err,"releasing buffer");

        gettimeofday( &now, NULL );
        elapsed = (now.tv_usec / 1000 + now.tv_sec * 1000) - 
            (start.tv_usec / 1000 + start.tv_sec * 1000);

        if (!use_stdout) {
            printf("\r%d frames (%lu ms)", ++numframes, elapsed);
            fflush(stdout);
        }
    }


    elapsed = (now.tv_usec / 1000 + now.tv_sec * 1000) - (start.tv_usec / 1000 + start.tv_sec * 1000);

    if (!use_stdout) {
        printf("\nFinished:\n");
        printf("\ttime elapsed: %lu ms - %4.1f fps\n", elapsed, (float)numframes/elapsed * 1000);
        printf("\tpprz: %d readings - %4.1f ps (%d errors, %d ignored)\n", parser.num, (float)parser.num/elapsed * 1000, parser.pprz_error, parser.ignored);
        printf("\tframe size: %ld\n", total_frame_size);
    }


    // close camera
    cleanup_and_exit(camera);
    dc1394_free (d);

    // kill thread
    parser.finished = TRUE;
    pthread_join(thread, NULL);
    fclose(fp);
	return 0;


}

