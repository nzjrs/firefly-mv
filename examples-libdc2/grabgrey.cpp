/**************************************************************************
 * 
 * Title:	grabgrey
 * Copyright:	(C) 2005 Don Murray donm@ptgrey.com 
 * Description:
 *    Get one grayscale image via DMA transfer using libdc1394 and store it 
 *    as a portable gray map (pgm).  Based on 'grab_gray_image' from libdc1394 
 *    examples.
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
#include <dc1394/control.h>
#include <dc1394/utils.h>
#include <stdlib.h>
#include <sys/time.h>


// define USE_1394B if you have a 1394B camera
//#define USE_1394B

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

   // enable this for verbose feature printouts
#if 0
   // report camera's features
   dc1394featureset_t 	features;
   if ( dc1394_get_camera_feature_set( camera, &features ) != DC1394_SUCCESS ) 
   {
      fprintf( stderr, "unable to get feature set\n");
   }
   else 
   {
      dc1394_print_feature_set( &features );
   }
#endif

   //  get the best video mode and highest framerate. This can be skipped
   //  if you already know which mode/framerate you want...
   // get video modes:
   dc1394video_modes_t 	video_modes;
   if ( dc1394_video_get_supported_modes( camera, &video_modes ) != DC1394_SUCCESS ) 
   {
      fprintf( stderr, "Can't get video modes\n" );
      cleanup_and_exit( camera );
   }

   // select highest res mode that is greyscale (MONO8)
   printf( "Searching for the highest resolution MONO8 mode available...\n" );
   dc1394video_mode_t 	video_mode;
   dc1394color_coding_t coding;
   for ( int i = video_modes.num-1; i >= 0; i-- ) 
   {
      // don't consider FORMAT 7 modes (i.e. "scalable")
      if ( !dc1394_is_video_mode_scalable( video_modes.modes[i] ) ) 
      {
	 dc1394_get_color_coding_from_video_mode( camera, video_modes.modes[i], &coding );
	 if ( coding == DC1394_COLOR_CODING_MONO8 ) 
	 {
	    video_mode = video_modes.modes[i];
	    break;
	 }
      }
   }

   // double check that we found a video mode  that is MONO8
   dc1394_get_color_coding_from_video_mode( camera, video_mode, &coding );
   if ( ( dc1394_is_video_mode_scalable( video_mode ) ) ||
	( coding != DC1394_COLOR_CODING_MONO8 ) ) 
   {
      fprintf( stderr, "Could not get a valid MONO8 mode\n" );
      cleanup_and_exit( camera );
   }

#ifdef USE_1394B
   dc1394_video_set_operation_mode( camera, DC1394_OPERATION_MODE_1394B );
   dc1394_video_set_iso_speed( camera, DC1394_ISO_SPEED_800 );
#else
   dc1394_video_set_iso_speed( camera, DC1394_ISO_SPEED_400 );
#endif
    
   // get highest framerate
   dc1394framerates_t 	framerates;
   dc1394framerate_t 	framerate;
   if ( dc1394_video_get_supported_framerates( camera, video_mode, &framerates ) != DC1394_SUCCESS ) 
   {
      fprintf(stderr,"Can't get framerates\n");
      cleanup_and_exit( camera );
   }
   framerate = framerates.framerates[ framerates.num-1 ];

   // setup capture
   printf( "Setting capture\n" );

   dc1394_video_set_mode( camera, video_mode );
   dc1394_video_set_framerate( camera, framerate );
   if ( dc1394_capture_setup_dma( camera, 8, DC1394_RING_BUFFER_LAST ) != DC1394_SUCCESS ) 
   {
      fprintf( stderr,"unable to setup camera-\n"
	       "check line %d of %s to make sure\n"
	       "that the video mode and framerate are\n"
	       "supported by your camera\n",
	       __LINE__,__FILE__ );
      fprintf( stderr, 
	       "video_mode = %d, framerate = %d\n"
	       "Check dc1394_control.h for the meanings of these values\n",
	       video_mode, framerate );
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


  printf( "Grabbing an image...\n" );
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
  
  unsigned int 	nCols;
  unsigned int 	nRows;
  dc1394_get_image_size_from_video_mode( camera, 
					 video_mode,
					 &nCols, 
					 &nRows );
  fprintf( imagefile, "P5\n%u %u 255\n", nCols, nRows );
  fwrite( (const char *)dc1394_capture_get_dma_buffer (camera), 1,
	  nCols*nRows, imagefile );
  fclose( imagefile );
  printf( "wrote: grey.pgm\n" );

  // close camera
  cleanup_and_exit( camera );

  return 0;
}
