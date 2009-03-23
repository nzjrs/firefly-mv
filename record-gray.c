// file: record-simple.c
// auth: Albert Huang
// date: October 10, 2005
//
// Records video from ladybug2 to disk.
//
// requires libdc1394 2.0+
//
// gcc -o record-simple record-simple.c -ldc1394_control

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/time.h>

#include <dc1394/dc1394.h>

#include "config.h"
#include "utils.h"

#define MY_VIDEO_MODE DC1394_VIDEO_MODE_640x480_MONO8

static void usage()
{
    printf( "usage: record <duration> <filename>\n\tspecify duration in seconds\n");
}

int main(int argc, char **argv)
{
    int duration = 0;
    char filename[1024] = { 0 };
    uint32_t width, height;
    dc1394_t * d;
    dc1394camera_t *camera;
    dc1394error_t err;
    dc1394video_frame_t *frame;

    if( argc < 3 ) {
        usage();
        exit(1);
    }

    duration = atoi(argv[1]);
    if( duration <= 0 ) {
        usage();
        exit(1);
    }

    strncpy( filename, argv[2], 1024 );
    FILE *fp = fopen( filename, "wb+");
    if( fp == NULL ) {
        perror("creating output file");
        exit(1);
    }

    d = dc1394_new ();
    if (!d)
        return 1;

    camera = dc1394_camera_new (d, MY_CAMERA_GUID);
    if (!camera)
        return 1;
    dc1394_camera_print_info(camera, stdout);
    
    printf("=======================\n\n\n");

    // setup capture
    dc1394_get_image_size_from_video_mode(camera, MY_VIDEO_MODE, &width, &height);
    err=setup_gray_capture(camera, MY_VIDEO_MODE);
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera),"Could not setup camera");

    err=dc1394_video_set_framerate(camera, DC1394_FRAMERATE_30);
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera),"setting framerate");

    // have the camera start sending us data
    err=dc1394_video_set_transmission(camera, DC1394_ON);
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera),"Could not start camera iso transmission");

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

        // write it to disk
        //fwrite(frame->image, frame->total_bytes, 1, fp);
        
        err=dc1394_capture_enqueue(camera,frame);
        DC1394_WRN(err,"releasing buffer");

        gettimeofday( &now, NULL );
        elapsed = (now.tv_usec / 1000 + now.tv_sec * 1000) - 
            (start.tv_usec / 1000 + start.tv_sec * 1000);

        printf("\r%d frames (%lu ms)", ++numframes, elapsed);
        fflush(stdout);
    }
    printf("\n");

    elapsed = (now.tv_usec / 1000 + now.tv_sec * 1000) - 
        (start.tv_usec / 1000 + start.tv_sec * 1000);
    printf("time elapsed: %lu ms - %4.1f fps\n", elapsed,
            (float)numframes/elapsed * 1000);


    // close camera
    cleanup_and_exit(camera);
    dc1394_free (d);

    fclose(fp);
	return 0;
}
