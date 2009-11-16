/*
 * Utility functions for working with pointgrey dc1394 cameras
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

#ifndef _UTILS_H_
#define _UTILS_H_

#include <inttypes.h>
#include <stdio.h>
#include <glib.h>
#include <dc1394/dc1394.h>

G_BEGIN_DECLS

typedef enum {
    GRAY =      'g',
    COLOR =     'c',
    FORMAT7 =   '7'
} show_mode_t;

/**
 * Cleanup function
 */
void cleanup_and_exit(dc1394camera_t *camera);

/**
 * Sets the camera to record color frames at the given video mode and color coding
 */
dc1394error_t setup_color_capture(
                dc1394camera_t *camera, 
                dc1394video_mode_t video_mode, 
                dc1394color_coding_t color_coding);

/**
 * Sets the camera to record gray frames at the given video mode
 */
dc1394error_t setup_gray_capture(
                dc1394camera_t *camera, 
                dc1394video_mode_t video_mode);

/**
 * Sets the camera framerate to the given floating point value, if supported
 */
dc1394error_t setup_framerate(
                dc1394camera_t *camera, 
                float framerate);

/**
 * If manual is TRUE then sets the camera exposure to value. If manual
 * is FALSE then puts the camera into automatic exposure mode.
 */
dc1394error_t setup_exposure(
                dc1394camera_t *camera, 
                uint8_t manual,
                uint32_t value);

/**
 * If manual is TRUE then sets the camera brightness to value. If manual
 * is FALSE then puts the camera into automatic brightness mode.
 */
dc1394error_t setup_brightness(
                dc1394camera_t *camera, 
                uint8_t manual,
                uint32_t value);

/**
 * Function and macro to setup camera from GOption command line arguments
 */
#define GOPTION_ENTRY_FORMAT(_format)                                                           \
      { "format", 'f', 0, G_OPTION_ARG_STRING, _format, "Format of image", "g,c,7" }
#define GOPTION_ENTRY_GUID(_guid)                                                               \
      { "guid", 'g', 0, G_OPTION_ARG_INT64, _guid, "Camera GUID", "0x456" }                     \

#define GOPTION_ENTRY_CAMERA_SETUP_ARGUMENTS(_guid, _framerate, _exposure, _brightness)         \
      { "guid", 'g', 0, G_OPTION_ARG_INT64, _guid, "Camera GUID", "0x456" },                    \
      { "framerate", 'F', 0, G_OPTION_ARG_DOUBLE, _framerate, "Framerate", "15.0" },            \
      { "exposure", 'e', 0, G_OPTION_ARG_INT, _exposure, "Exposure (<0 = Auto)", "13" },        \
      { "brightness", 'b', 0, G_OPTION_ARG_INT, _brightness, "Brightness (<0 = Auto)", "34" }
dc1394error_t setup_from_command_line(
                dc1394camera_t *camera, 
                float framerate, 
                int exposure,
                int brightness);

/**
 * Prints various information about the mode the camera is in
 */
void print_video_mode_info( dc1394camera_t *camera , dc1394video_mode_t mode);

/**
 * Foo
 */
long write_frame_binary_header(dc1394video_frame_t *frame, FILE *fp);

/**
 * Foo
 */
long read_frame_binary_header(dc1394video_frame_t *frame, FILE *fp);

long write_frame(dc1394video_frame_t *frame, FILE *fp);

long read_frame(dc1394video_frame_t *frame, FILE *fp);

long write_frame_with_extras(dc1394video_frame_t *frame, FILE *fp, uint8_t *extra, uint8_t nextra);

long read_frame_with_extras(dc1394video_frame_t *frame, FILE *fp, uint8_t *extra, uint8_t nextra);

void print_frame_info(dc1394video_frame_t *frame);

void app_exit(int code, GOptionContext *context, const char *msg);

G_END_DECLS

#endif
