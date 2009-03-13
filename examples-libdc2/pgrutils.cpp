

#include "pgrutils.h"
#include <stdio.h>
#include <stdlib.h>



dc1394error_t
getBayerTile( dc1394camera_t* camera,
	      dc1394color_filter_t* bayerPattern )
{

   quadlet_t value;
   dc1394error_t err;

   // query register 0x1040
   // This register is an advanced PGR register called BAYER_TILE_MAPPING
   // For more information check the PGR IEEE-1394 Digital Camera Register Reference
   err = GetCameraControlRegister( camera, 0x1040, &value );
   if ( err != DC1394_SUCCESS )
   {
      return err;
   }
   
   // Ascii R = 52 G = 47 B = 42 Y = 59
   switch( value )
   {
      default:
      case 0x59595959:	// YYYY
	 // no bayer
	 *bayerPattern = (dc1394color_filter_t) 0;
	 break;
      case 0x52474742:	// RGGB
	 *bayerPattern = DC1394_COLOR_FILTER_RGGB;
	 break;
      case 0x47425247:	// GBRG
	 *bayerPattern = DC1394_COLOR_FILTER_GBRG;
	 break;
      case 0x47524247:	// GRBG
	 *bayerPattern = DC1394_COLOR_FILTER_GRBG;
	 break;
      case 0x42474752:	// BGGR
	 *bayerPattern = DC1394_COLOR_FILTER_BGGR;
	 break;
   }

   return err;
}
			

