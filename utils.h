#ifndef _UTILS_H_
#define _UTILS_H_

#include <dc1394/dc1394.h>

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
 * Prints various information about the mode the camera is in
 */
void print_video_mode_info( dc1394camera_t *camera , dc1394video_mode_t mode);

#endif
