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

#include <dc1394/dc1394.h>

#include "config.h"
#include "utils.h"

#define s2s(status) ((status)==DC1394_SUCCESS?"OK":"FAILURE")

//USB Cameras only work with 1394A
#define MY_CAMERA_OPERATION_MODE DC1394_OPERATION_MODE_1394B

static void usage()
{
    printf( "usage: record <duration> <filename>\n"
            "  specify duration in seconds\n");
}

int main(int argc, char **argv)
{
    int duration = 0;
    char filename[1024] = { 0 };
    unsigned int speed, framerate, mode, width, height;
    dc1394_t * d;
    dc1394camera_t *camera;
    dc1394error_t err;
    dc1394operation_mode_t opmode;

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

    // Setup format 7
    dc1394_video_set_iso_speed(camera, DC1394_ISO_SPEED_400);
    dc1394_video_set_mode(camera, DC1394_VIDEO_MODE_FORMAT7_0);
    err = dc1394_format7_set_roi(
                            camera, 
                            DC1394_VIDEO_MODE_FORMAT7_0,
                            DC1394_COLOR_CODING_MONO8,
                            DC1394_USE_MAX_AVAIL,      // use max packet size
                            0, 0,                      // left, top
                            MY_CAMERA_W, MY_CAMERA_H); // width, height
    DC1394_ERR_RTN(err,"Unable to set Format7 mode 0.\nEdit the example file manually to fit your camera capabilities");

    err=dc1394_capture_setup(camera, 4, DC1394_CAPTURE_FLAGS_DEFAULT);
    DC1394_ERR_CLN_RTN(err, dc1394_camera_free(camera), "Error capturing");

    // initialize this camera
    status = dc1394_camera_reset(camera);
    DC1394_ERR_CHK(status, "resetting camera");
	

    // set 1394b mode
    status = dc1394_video_get_operation_mode( camera, &opmode );
    DC1394_ERR_CHK(status, "retrieving operation mode");
    printf("get operation mode (%s): %u\n", s2s(status), opmode);

    printf("setting operation mode...\n");
    status = dc1394_video_set_operation_mode( camera, MY_CAMERA_OPERATION_MODE );
    DC1394_ERR_CHK(status, "retrieving operation mode");

    status = dc1394_video_get_operation_mode( camera, &opmode );
    DC1394_ERR_CHK(status, "retrieving operation mode");
    printf("get operation mode (%s): %u\n", s2s(status), opmode);

    // start data transmission
    status = dc1394_video_set_transmission(camera, DC1394_ON);
    DC1394_ERR_CHK(status, "enabling video transmission");

    printf("setting speed...\n");
    status = dc1394_video_set_iso_speed(camera, DC1394_SPEED_800);
    DC1394_ERR_CHK(status, "setting speed");

    status = dc1394_video_get_iso_speed(camera, &speed);
    DC1394_ERR_CHK(status, "getting channel and speed");
    printf("get speed: %u\n", speed);

    // get and set framerate
    status = dc1394_video_get_framerate(camera, &framerate);
    DC1394_ERR_CHK(status, "getting framerate");
    printf("get framerate: %u\n", framerate);

    printf("setting framerate...\n");
    status = dc1394_video_set_framerate(camera, DC1394_FRAMERATE_30;);
    DC1394_ERR_CHK(status, "setting framerate");

    status = dc1394_video_get_framerate(camera, &framerate);
    DC1394_ERR_CHK(status, "getting framerate");
    printf("get framerate: %u\n", framerate);

    // setup capture
    status = dc1394_dma_setup_format7_capture(
                camera, 0, DC1394_MODE_FORMAT7_0, speed,
                DC1394_USE_MAX_AVAIL, 
                0, 0, DC1394_USE_MAX_AVAIL, DC1394_USE_MAX_AVAIL, 
                10, 0, NULL);

    // check the image size for the capture
    status = dc1394_format7_get_image_size(camera, mode, &width, &height);
    DC1394_ERR_CHK(status, "querying image size");
    printf("capture image width: %d height: %d\n", width, height);

    // check the image size in bytes
    uint64_t totalbytes;
    status = dc1394_format7_get_total_bytes(camera, mode, &totalbytes);
    DC1394_ERR_CHK(status, "querying bytes per frame");
    printf("bytes per frame: %llu\n", totalbytes);

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
        status = dc1394_dma_capture(&camera, 1, 0);
        DC1394_ERR_CHK(status,"capturing frame");

        // write it to disk
        fwrite(camera->capture.capture_buffer, totalbytes, 1, fp);
        
        status = dc1394_dma_done_with_buffer( camera );
        DC1394_ERR_CHK(status,"releasing buffer");

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


    // release capture resources
    dc1394_dma_unlisten(camera);
    dc1394_dma_release_camera(camera);

    // stop data transmission
    status = dc1394_video_set_transmission(camera, DC1394_OFF);
    DC1394_ERR_CHK(status, "disabling video transmission");

    dc1394_free_camera( camera );

    fclose(fp);
	return 0;
}
