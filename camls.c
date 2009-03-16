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
#include <dc1394/dc1394.h>
#include <stdlib.h>
#include <inttypes.h>

#define print_case(A) case A: printf(#A ""); break;

/*-----------------------------------------------------------------------
 *  Prints the type of format to standard out
 *-----------------------------------------------------------------------*/
static void print_format( uint32_t format )
{
    switch( format ) {
        print_case(DC1394_VIDEO_MODE_160x120_YUV444);
        print_case(DC1394_VIDEO_MODE_320x240_YUV422);
        print_case(DC1394_VIDEO_MODE_640x480_YUV411);
        print_case(DC1394_VIDEO_MODE_640x480_YUV422);
        print_case(DC1394_VIDEO_MODE_640x480_RGB8);
        print_case(DC1394_VIDEO_MODE_640x480_MONO8);
        print_case(DC1394_VIDEO_MODE_640x480_MONO16);
        print_case(DC1394_VIDEO_MODE_800x600_YUV422);
        print_case(DC1394_VIDEO_MODE_800x600_RGB8);
        print_case(DC1394_VIDEO_MODE_800x600_MONO8);
        print_case(DC1394_VIDEO_MODE_1024x768_YUV422);
        print_case(DC1394_VIDEO_MODE_1024x768_RGB8);
        print_case(DC1394_VIDEO_MODE_1024x768_MONO8);
        print_case(DC1394_VIDEO_MODE_800x600_MONO16);
        print_case(DC1394_VIDEO_MODE_1024x768_MONO16);
        print_case(DC1394_VIDEO_MODE_1280x960_YUV422);
        print_case(DC1394_VIDEO_MODE_1280x960_RGB8);
        print_case(DC1394_VIDEO_MODE_1280x960_MONO8);
        print_case(DC1394_VIDEO_MODE_1600x1200_YUV422);
        print_case(DC1394_VIDEO_MODE_1600x1200_RGB8);
        print_case(DC1394_VIDEO_MODE_1600x1200_MONO8);
        print_case(DC1394_VIDEO_MODE_1280x960_MONO16);
        print_case(DC1394_VIDEO_MODE_1600x1200_MONO16);
        print_case(DC1394_VIDEO_MODE_EXIF);
        print_case(DC1394_VIDEO_MODE_FORMAT7_0);
        print_case(DC1394_VIDEO_MODE_FORMAT7_1);
        print_case(DC1394_VIDEO_MODE_FORMAT7_2);
        print_case(DC1394_VIDEO_MODE_FORMAT7_3);
        print_case(DC1394_VIDEO_MODE_FORMAT7_4);
        print_case(DC1394_VIDEO_MODE_FORMAT7_5);
        print_case(DC1394_VIDEO_MODE_FORMAT7_6);
        print_case(DC1394_VIDEO_MODE_FORMAT7_7);
    default:
        dc1394_log_error("Unknown format: %i\n", format);
        exit(1);
    }

}

/*-----------------------------------------------------------------------
 *  Prints the type of color encoding
 *-----------------------------------------------------------------------*/
static void print_color_coding( uint32_t color_id )
{
    switch( color_id ) {
        print_case(DC1394_COLOR_CODING_MONO8);
        print_case(DC1394_COLOR_CODING_YUV411);
        print_case(DC1394_COLOR_CODING_YUV422);
        print_case(DC1394_COLOR_CODING_YUV444);
        print_case(DC1394_COLOR_CODING_RGB8);
        print_case(DC1394_COLOR_CODING_MONO16);
        print_case(DC1394_COLOR_CODING_RGB16);
        print_case(DC1394_COLOR_CODING_MONO16S);
        print_case(DC1394_COLOR_CODING_RGB16S);
        print_case(DC1394_COLOR_CODING_RAW8);
        print_case(DC1394_COLOR_CODING_RAW16);
    default:
        dc1394_log_error("Unknown color coding = %d\n",color_id);
        exit(1);
    }
}

static void print_color_filter( dc1394color_filter_t color)
{
    switch ( color ) {
        print_case(DC1394_COLOR_FILTER_RGGB);
        print_case(DC1394_COLOR_FILTER_GBRG);
        print_case(DC1394_COLOR_FILTER_GRBG);
        print_case(DC1394_COLOR_FILTER_BGGR);
    default:
        dc1394_log_error("Unknown color filter = %d\n",color);
        exit(1);
    }
}


/*-----------------------------------------------------------------------
 *  Prints various information about the mode the camera is in
 *-----------------------------------------------------------------------*/
static void print_mode_info( dc1394camera_t *camera , uint32_t mode )
{
    int j;
    dc1394error_t err;

    printf("Mode: ");
    print_format(mode);
    printf("\n");

    if ((mode == DC1394_VIDEO_MODE_FORMAT7_0) ||
        (mode == DC1394_VIDEO_MODE_FORMAT7_1) ||
        (mode == DC1394_VIDEO_MODE_FORMAT7_2) ||
        (mode == DC1394_VIDEO_MODE_FORMAT7_3) ||
        (mode == DC1394_VIDEO_MODE_FORMAT7_4) ||
        (mode == DC1394_VIDEO_MODE_FORMAT7_5) ||
        (mode == DC1394_VIDEO_MODE_FORMAT7_6) ||
        (mode == DC1394_VIDEO_MODE_FORMAT7_7)) {
        dc1394format7mode_t f7mode;

        err=dc1394_format7_get_mode_info(camera, mode, &f7mode);
        if (err) {
            DC1394_ERR(err,"Could not format 7 information");
        }
        else {
            printf( "Image Sizes:\n"
                    "  size = %ix%i\n"
                    "  max = %ix%i\n"
                    "  pixels = %i\n", 
                    f7mode.size_x, f7mode.size_y,
                    f7mode.max_size_x, f7mode.max_size_y,
                    f7mode.pixnum);

            printf( "Color:\n");            
            for (j=0; j<f7mode.color_codings.num; j++) {
                printf("  [%d] coding = ", j);
                print_color_coding(f7mode.color_codings.codings[j]);
                printf("\n");
            }
            printf("  filter = ");
            print_color_filter(f7mode.color_filter);
            printf("\n");
        }

    } else {
        dc1394framerates_t framerates;
        err=dc1394_video_get_supported_framerates(camera,mode,&framerates);
        if (err) {
            DC1394_ERR(err,"Could not get frame rates");
        } else {
            printf("Frame Rates:\n");
            for( j = 0; j < framerates.num; j++ ) {
                uint32_t rate = framerates.framerates[j];
                float f_rate;
                dc1394_framerate_as_float(rate,&f_rate);
                printf("  [%d] rate = %f\n",j,f_rate );
            }
        }
    }

}

int main(int argc, char *argv[])
{
    unsigned int i;
    dc1394_t * d;
    dc1394camera_list_t * list;
    dc1394error_t err;
    dc1394featureset_t features;

    d = dc1394_new ();
    if (!d)
        return 1;
    err=dc1394_camera_enumerate (d, &list);
    DC1394_ERR_RTN(err,"Failed to enumerate cameras");

    if (list->num == 0) {
        dc1394_log_error("No cameras found");
        return 1;
    }

    for (i = 0; i < list->num; i++) {
        dc1394camera_t *camera = dc1394_camera_new (d, list->ids[0].guid);

        if (camera) {
            unsigned int j;
            dc1394video_modes_t modes;
            
            /* Print hardware information of camera */
            dc1394_camera_print_info(camera, stdout);

            /* Print supported camera features */
            err=dc1394_feature_get_all(camera,&features);
            if (err!=DC1394_SUCCESS) {
                dc1394_log_warning("Could not get feature set");
            } else {
                dc1394_feature_print_all(&features, stdout);
            }


            /* Print a list of supported modes for this camera */
            printf("------ Supported Video Modes ------\n");

            err=dc1394_video_get_supported_modes(camera, &modes);
            DC1394_ERR_RTN(err,"Could not get list of modes");

            for (j = 0; j < modes.num; j++) {
                print_mode_info(camera, modes.modes[j]);
            }

            dc1394_camera_free(camera);
        }
    }
    dc1394_camera_free_list (list);
    dc1394_free (d);

    return 0;
}
