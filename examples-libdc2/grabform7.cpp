/**************************************************************************
 * 
 * Title:	grabform7
 * Copyright:	(C) 2006 Don Murray donm@ptgrey.com 
 * Description:
 *    Get one grayscale image via DMA transfer using libdc1394 and store it 
 *    as a portable gray map (pgm).  Based on 'grab_gray_image' from libdc1394 
 *    examples.  The grab is made in format7 mode, aka "custom image" mode, so 
 *    any image size can be specified.  This is set up in the top of the file.
 *    
 *-------------------------------------------------------------------------
 *     License: LGPL
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *************************************************************************/

#include <stdio.h>
#include <dc1394/dc1394_control.h>
#include <dc1394/dc1394_utils.h>
#include <stdlib.h>
#include <sys/time.h>


// These values are designed for a Scorpion 2megapixel
// For other cameras, adjust to values where the ROI will be inside the 
// sensor area
const int nImageWidth	= 1400;
const int nImageHeight	= 960;
const int nLeftOffset	= 0;
const int nTopOffset	= 0;



/*==== cleanup_and_exit()
 * This is called when the program exits and destroys the existing connections to the
 * 1394 drivers
 *====*/
void 
cleanup_and_exit( dc1394camera_t* camera ) 
{
   dc1394_capture_stop( camera );
   dc1394_video_set_transmission( camera, DC1394_OFF );
   dc1394_free_camera( camera );
   exit( 1 );
}

int main( int argc, char *argv[] ) 
{
   dc1394camera_t* 	camera;
   int			err;

   // Find cameras on the 1394 buses
   uint_t 		nCameras;
   dc1394camera_t**	cameras=NULL;
   err = dc1394_find_cameras( &cameras, &nCameras );

   if ( err != DC1394_SUCCESS ) 
   {
      fprintf( stderr, "Unable to look for cameras\n\n"
	       "Please check \n"
	       "  - if the kernel modules `ieee1394',`raw1394' and `ohci1394' are loaded \n"
	       "  - if you have read/write access to /dev/raw1394\n\n");
      exit(1);
   }

   //  get the camera nodes and describe them as we find them
   if ( nCameras < 1 ) 
   {
      fprintf( stderr, "No cameras found!\n");
      exit(1);
   }
   camera = cameras[0];
   printf( "There were %d cameras found attached to your PC\n", nCameras );
   printf( "Working with the first camera\n" );
  
   // free the other cameras
   for ( unsigned int i = 1; i < nCameras; i++ )
      dc1394_free_camera( cameras[i] );
   free(cameras);

   dc1394_video_set_iso_speed( camera, DC1394_ISO_SPEED_400 );

   // NOTE: for most PGR cameras, format 7, mode 0 is ROI mode.  However, this should
   // be verified by checking the camera technical manual
   dc1394_video_set_mode( camera, DC1394_VIDEO_MODE_FORMAT7_0 );

   err = dc1394_format7_set_roi( camera,
				 DC1394_VIDEO_MODE_FORMAT7_0,
				 DC1394_COLOR_CODING_MONO8,
				 DC1394_USE_MAX_AVAIL, // bytes per packet
				 nLeftOffset, 
				 nTopOffset,
				 nImageWidth,
				 nImageHeight );

   // setup capture
   printf( "Setting capture\n" );

   if ( dc1394_capture_setup_dma( camera, 8, DC1394_RING_BUFFER_LAST ) != DC1394_SUCCESS ) 
   {
      fprintf( stderr,"unable to setup camera-\n"
	       "check line %d of %s to make sure\n"
	       "that the video mode and framerate are\n"
	       "supported by your camera\n",
	       __LINE__,__FILE__ );
      cleanup_and_exit( camera );
   }

   // have the camera start sending us data
   printf( "Start transmission\n" );
   if ( dc1394_video_set_transmission( camera, DC1394_ON ) != DC1394_SUCCESS ) 
   {
      fprintf( stderr, "Unable to start camera iso transmission\n" );
      cleanup_and_exit( camera );
   }

   printf( "Waiting for transmission... \n" );
   //  Sleep untill the camera has a transmission
   dc1394switch_t status = DC1394_OFF;

   for ( int i = 0; i <= 5; i++ )
   {
      usleep(50000);
      if ( dc1394_video_get_transmission( camera, &status ) != DC1394_SUCCESS ) 
      {
	 fprintf( stderr, "Unable to get transmision status\n" );
	 cleanup_and_exit( camera );
      }
      if ( status != DC1394_OFF )
	 break;

      if( i == 5 ) 
      {
	 fprintf(stderr,"Camera doesn't seem to want to turn on!\n");
	 cleanup_and_exit( camera );
      }
   }

   printf( "Capturing an image\n" );
   //  capture one frame
   if ( dc1394_capture_dma( &camera, 1, DC1394_VIDEO1394_WAIT )!= DC1394_SUCCESS) 
   {
      fprintf( stderr, "Unable to capture a frame\n" );
      cleanup_and_exit( camera );
   }
      
   printf( "Stop transmission\n" );
   //  Stop data transmission
   if ( dc1394_video_set_transmission( camera, DC1394_OFF ) != DC1394_SUCCESS ) 
   {
      printf( "Couldn't stop the camera?\n" );
   }
   
   // save image as 
   FILE* imagefile;
   imagefile = fopen( "grey.pgm", "w" );
   if( imagefile == NULL) 
   {
      perror( "Can't create 'grey.pgm' " );
     cleanup_and_exit( camera );
   }
   
   fprintf( imagefile, "P5\n%u %u 255\n", nImageWidth, nImageHeight );
   fwrite( (const char *)dc1394_capture_get_dma_buffer (camera), 1,
	   nImageWidth*nImageHeight, imagefile );
   fclose( imagefile );
   printf( "wrote: grey.pgm\n" );
   
   // close camera
   cleanup_and_exit( camera );
   
   return 0;
}
