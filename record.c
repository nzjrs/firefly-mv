/*
 * Record video from the dc1394 camera
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
 *    Writes successive image frames into a single binary file for the
 *    specified duration at the given framerate. Use dc1394-play to replay
 *    this binary file later.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/time.h>

#include <glib.h>
#include <dc1394/dc1394.h>

#include "camera.h"
#include "utils.h"

int main(int argc, char **argv)
{
    FILE *fp = NULL;
    unsigned char use_stdout = 0;
    uint32_t width, height;
    dc1394_t * d;
    dc1394camera_t *camera;
    dc1394error_t err;
    dc1394video_frame_t *frame;

    /* Options */
    show_mode_t show;
    char *format;
    char *filename;
    double framerate;
    int exposure, brightness, duration, i;
    guint64 guid;

    /* Option parsing */
    GError *error = NULL;
    GOptionContext *context;
    GOptionEntry entries[] =
    {
      GOPTION_ENTRY_FORMAT(&format),
      { "output-filename", 'o', 0, G_OPTION_ARG_FILENAME, &filename, "Output filename", "FILE" },
      { "duration", 'd', 0, G_OPTION_ARG_INT, &duration, "Seconds to record", NULL },
      GOPTION_ENTRY_CAMERA_SETUP_ARGUMENTS(&guid, &framerate, &exposure, &brightness),
      { NULL }
    };

    context = g_option_context_new("- Firefly MV Camera Recorder");
    g_option_context_add_main_entries (context, entries, NULL);

    /* Defaults */
    guid = MY_CAMERA_GUID;
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
    if (filename == NULL)
        app_exit(2, context, "Error: You must supply a filename");
    if (duration <= 0)
        app_exit(3, context, "Error: You must supply a duration");

    if (format && format[0])
        show = format[0];

    if (filename[0] == '-') {
        use_stdout = 1;
        fp = stdout;
    } else {
        fp = fopen( filename, "wb+");
    }

    if( fp == NULL ) {
        app_exit(4, NULL, "Error creating output file");
    }

    d = dc1394_new ();
    if (!d)
        app_exit(5, NULL, "Could not initialize libdc1394");

    camera = dc1394_camera_new (d, guid);
    if (!camera)
        app_exit(6, context, "Could not find or initialize camera");

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

        write_frame(frame, fp);
        
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

    elapsed = (now.tv_usec / 1000 + now.tv_sec * 1000) - 
        (start.tv_usec / 1000 + start.tv_sec * 1000);

    if (!use_stdout) {
        printf("\n");
        printf("time elapsed: %lu ms - %4.1f fps\n", elapsed,
                (float)numframes/elapsed * 1000);
    }


    // close camera
    cleanup_and_exit(camera);
    dc1394_free (d);

    fclose(fp);
	return 0;
}
