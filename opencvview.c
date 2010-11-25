/*
 * Use Gtk+ to show live video from the dc1394 camera
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
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>

#include <cv.h>
#include <highgui.h>
#include <glib.h>
#include <dc1394/dc1394.h>

#include "camera.h"
#include "utils.h"
#include "opencvutils.h"

int main(int argc, char *argv[])
{
    dc1394_t        *d;
    dc1394camera_t  *camera;
    IplImage        *frame;
    dc1394error_t   err;
    guint64         guid = 0x00b09d0100818d56LL;

    d = dc1394_new ();
    if (!d)
        g_critical("Could not create dc1394 context");

    camera = dc1394_camera_new (d, guid);
    if (!camera)
        g_critical("Could not create dc1394 camera");

    // setup
    err = setup_gray_capture(camera, DC1394_VIDEO_MODE_640x480_MONO8);
    DC1394_ERR_CLN_RTN(err, cleanup_and_exit(camera), "Could not setup camera");

    // enable camera
    err = dc1394_video_set_transmission(camera, DC1394_ON);
    DC1394_ERR_CLN_RTN(err, cleanup_and_exit(camera), "Could not start camera iso transmission");

	cvNamedWindow("Input", CV_WINDOW_AUTOSIZE);

    while (1) {
        frame = dc1394_capture_get_iplimage(camera);
        if (frame)
		    cvShowImage("Input", frame);
        if( cvWaitKey(50) >= 0 )
            break;
        if (frame)
            cvReleaseImage(&frame);
	}

    cvDestroyWindow("Input");

    // stop data transmission
    err = dc1394_video_set_transmission(camera, DC1394_OFF);
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera),"Could not stop the camera");

    // close camera
    cleanup_and_exit(camera);
    dc1394_free (d);

    return 0;
}
