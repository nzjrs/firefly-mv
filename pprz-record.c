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

#include "config.h"
#include "utils.h"
#include "serial.h"
#include "ppzutils.h"

#define SERIAL_PORT "/dev/ttyACM0"
#define SERIAL_BAUD 57600

#define UNINIT 0
#define GOT_STX 1
#define GOT_LENGTH 2
#define GOT_PAYLOAD 3
#define GOT_CRC1 4

typedef struct __parser
{
    int status;
    int num;
    int ignored;
    uint8_t pprz_payload_len;
    uint8_t payload_idx;
    uint8_t pprz_msg_received;
    int pprz_ovrn;
    int pprz_error;
    char payload[256];
    uint8_t ck_a;
    uint8_t ck_b;

    /* Thread communication */
    uint8_t finished;
    char data[AHRS_PAYLOAD_LEN];
    int serial;
} PprzParser_t;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void ppz_parse_serial (PprzParser_t *parser)
{
    uint8_t c;
    int i;

    i = serial_read_port(parser->serial, &c, 1);
    //if (i != 1)
    //    printf("---------------------------");

    if (i) {
        //Adapted from pprz_transport.h
        //printf("%d %x %d\n", c == 0x99, c, parser->status);
        switch (parser->status) {
            case UNINIT:
                if (c == STX)
                    parser->status++;
                break;
            case GOT_STX:
                if (parser->pprz_msg_received) {
                    parser->pprz_ovrn++;
                    goto error;
                }
                parser->pprz_payload_len = c-4; // Counting STX, LENGTH and CRC1 and CRC2 
                parser->ck_a = parser->ck_b = c;
                parser->status++;
                parser->payload_idx = 0;
                break;
            case GOT_LENGTH:
                parser->payload[parser->payload_idx] = c;
                parser->ck_a += c; parser->ck_b += parser->ck_a;
                parser->payload_idx++;
                if (parser->payload_idx == parser->pprz_payload_len)
                    parser->status++;
                break;
            case GOT_PAYLOAD:
                if (c != parser->ck_a)
                    goto error;
                parser->status++;
                break;
            case GOT_CRC1:
                if (c != parser->ck_b)
                    goto error;
                parser->pprz_msg_received = TRUE;
                goto restart;
            default:
                //printf(".............................");
                break;
        }
    return;
    error:
        parser->pprz_error++;
        //printf("!");
    restart:
        parser->status = UNINIT;
    }
    return;
}

void *parse_pppz_thread(void *ptr)
{
    PprzParser_t *parser = (PprzParser_t *)ptr;

    parser->serial = serial_open(SERIAL_PORT, SERIAL_BAUD);
    serial_set_blocking(parser->serial);
    parser->status = 0;
    parser->pprz_msg_received = FALSE;
    parser->finished = FALSE;
    parser->num = 0;
    parser->ignored = 0;
    parser->pprz_ovrn = 0;
    parser->pprz_error = 0;

    while(parser->finished == FALSE)
    {
        ppz_parse_serial (parser);
        if (parser->pprz_msg_received) {
            //int32_t theta;

            parser->num++;
            parser->pprz_msg_received = FALSE;
            //theta = DL_BOOZ2_AHRS_EULER_body_theta(parser->payload);

            if ((uint8_t)parser->payload[1] == 0x9c && parser->pprz_payload_len == AHRS_PAYLOAD_LEN) {

                //printf("%2.2f\n",(float)(theta * 0.0139882));
                   pthread_mutex_lock( &mutex );
                    memcpy(parser->data, parser->payload, parser->pprz_payload_len);
               pthread_mutex_unlock( &mutex );
            } else {
                parser->ignored++;
            }
        }
    }

    close(parser->serial);
    return 0;
}

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

    /* Options */
    show_mode_t show;
    char *format;
    char *filename;
    double framerate;
    int exposure, brightness, duration, i;

    char data[AHRS_PAYLOAD_LEN];

    /* Option parsing */
    GError *error = NULL;
    GOptionContext *context;
    GOptionEntry entries[] =
    {
      { "format", 'f', 0, G_OPTION_ARG_STRING, &format, "Format of image", "g,c,7" },
      { "output-filename", 'o', 0, G_OPTION_ARG_FILENAME, &filename, "Output filename", "FILE" },
      { "duration", 'd', 0, G_OPTION_ARG_INT, &duration, "Seconds to record", NULL },
      GOPTION_ENTRY_CAMERA_SETUP_ARGUMENTS(&framerate, &exposure, &brightness),
      { NULL }
    };

    context = g_option_context_new("- Firefly MV Camera Recorder");
    g_option_context_add_main_entries (context, entries, NULL);

    /* Defaults */
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
        exit(2);
    }
    if (duration <= 0) {
        printf( "Error: You must supply a duration\n%s", 
                g_option_context_get_help(context, TRUE, NULL));
        exit(3);
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
                "  Duration   = %d\n"
                "  Format     = %c\n"
                "  Framerate  = %f\n"
                "  Exposure   = %d\n"
                "  Brightness = %d\n\n"
                "Recording:\n",
                duration,show,framerate,exposure,brightness);
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

    // start the serial thread
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

        pthread_mutex_lock( &mutex );
        memcpy(data, parser.data, AHRS_PAYLOAD_LEN);      
        pthread_mutex_unlock( &mutex );

        write_frame_with_extras(frame, fp, data, AHRS_PAYLOAD_LEN);
//        printf("%2.2f\n",(float)((DL_BOOZ2_AHRS_EULER_body_theta(data)) * 0.0139882));
        
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

