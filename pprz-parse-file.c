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
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <glib.h>
#include <dc1394/dc1394.h>

#include "camera.h"
#include "utils.h"
#include "ppzutils.h"

int main( int argc, char *argv[])
{
    char                *filename, *csvline;
    FILE                *fp;
    int                 i;
    long                total_frame_size;
    dc1394video_frame_t frame;
    show_mode_t         show;

    /* This structure, containing the payload and other timely information
       is read from disk with the image file */
    PprzRecord_t        record;

    /* Option parsing */
    GError              *error = NULL;
    GOptionContext      *context;
    GOptionEntry entries[] =
    {
      { "input-filename", 'i', 0, G_OPTION_ARG_FILENAME, &filename, "Input filename", "FILE" },
      { NULL }
    };

    context = g_option_context_new("- Play recorded imu data");
    g_option_context_add_main_entries (context, entries, NULL);

    fp = NULL;
    filename = NULL;

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

    fp = fopen(filename, "rb");
    if( fp == NULL ) { 
        perror("opening file");
        exit(1);
    }

    // read the first frame
    total_frame_size = read_frame_with_extras(&frame, fp, (uint8_t *)&record, PPRZ_RECORD_SIZE );
    if (frame.color_coding == DC1394_COLOR_CODING_MONO8)
        show = GRAY;
    else if (frame.color_coding == DC1394_COLOR_CODING_RGB8)
        show = COLOR;
    else if (frame.color_coding == DC1394_COLOR_CODING_RAW8)
        show = FORMAT7;
    else {
        perror("invalid color coding");
        exit(1);
    }

    /* go back to start of file */
    i = 0;
    fseek(fp, 0, SEEK_SET);
    while (!feof(fp)) 
    {
        if (frame.image) {
            free(frame.image);
            frame.image = NULL;
        }

        read_frame_with_extras( &frame, fp , (uint8_t *)&record, PPRZ_RECORD_SIZE );

        csvline = g_strdup_printf("%d, %llu, " CSV_FORMAT,
                            i,
                            record.timestamp,
                            (float)(MESSAGE_EMAV_STATE_GET_FROM_BUFFER_ax(record.data) * 0.0009766),
                            (float)(MESSAGE_EMAV_STATE_GET_FROM_BUFFER_ay(record.data) * 0.0009766),
                            (float)(MESSAGE_EMAV_STATE_GET_FROM_BUFFER_az(record.data) * 0.0009766),
                            (float)(MESSAGE_EMAV_STATE_GET_FROM_BUFFER_gp(record.data) * 0.0139882),
                            (float)(MESSAGE_EMAV_STATE_GET_FROM_BUFFER_gq(record.data) * 0.0139882),
                            (float)(MESSAGE_EMAV_STATE_GET_FROM_BUFFER_gr(record.data) * 0.0139882),
                            (float)(MESSAGE_EMAV_STATE_GET_FROM_BUFFER_body_phi(record.data) * 0.0139882),
                            (float)(MESSAGE_EMAV_STATE_GET_FROM_BUFFER_body_theta(record.data) * 0.0139882),
                            (float)(MESSAGE_EMAV_STATE_GET_FROM_BUFFER_body_psi(record.data) * 0.0139882));
        fwrite(csvline, sizeof(char), strlen(csvline), stdout);
        free(csvline);

        i++;
    }
    printf("Read %d frames (%ld)\n", i, total_frame_size);

    fclose(fp);

    return 0;
}
