#ifndef PGRUTILS_LIBDC_2_H
#define PGRUTILS_LIBDC_2_H

#include <dc1394/dc1394_control.h>
#include <dc1394/dc1394_register.h>

dc1394error_t
getBayerTile( dc1394camera_t* camera,
	      dc1394color_filter_t* bayerPattern );
			

#endif // PGRUTILS_LIBDC_2_H
