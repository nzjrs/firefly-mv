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

#include <dc1394/dc1394.h>

#include "utils.h"

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
        dc1394camera_t *camera = dc1394_camera_new (d, list->ids[i].guid);

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
                print_video_mode_info(camera, modes.modes[j]);
            }

            dc1394_camera_free(camera);
        }
    }
    dc1394_camera_free_list (list);
    dc1394_free (d);

    return 0;
}
