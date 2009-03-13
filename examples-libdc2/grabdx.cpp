/**************************************************************************
 * 
 * Title:	grabdx
 * Copyright:	(C) 2005 Don Murray donm@ptgrey.com 
 * Description:
 *    Get one grayscale image via DMA transfer using libdc1394 and store it 
 *    as a portable gray map (pgm).  Based on 'grab_gray_image' from libdc1394 
 *    examples.  This program is targetted to the Dragonfly Express 200 FPS camera
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
#include <stdlib.h>
#include <sys/time.h>



// set GRAB_SPEED to an integer value of the percentage of full speed.  I.e. 100% = 200 fps
// 50% = 100 fps, etc.
#define DEFAULT_GRAB_SPEED 100
#define BYTES_PER_PACKET_200_FPS 8160
#define DEFAULT_N_IMAGES_TO_GRAB 1000


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
   int nImagesToGrab 	= DEFAULT_N_IMAGES_TO_GRAB;
   int nGrabSpeed 	= DEFAULT_GRAB_SPEED;

   switch( argc )
   {
      case 3:
	 nImagesToGrab = atoi( argv[2] );
      case 2:
	 nGrabSpeed = atoi( argv[1] );
      default:
      case 1:
	 // go with defaults
	 break;
   }

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

   dc1394_video_set_operation_mode( camera, DC1394_OPERATION_MODE_1394B );
   dc1394_video_set_iso_speed( camera, DC1394_ISO_SPEED_800 );
   dc1394_video_set_mode( camera, DC1394_VIDEO_MODE_FORMAT7_0 );

   if ( nGrabSpeed <= 0 || nGrabSpeed > 100 )
   {
      printf( "GrabSpeed = %d - must be between 0 and 100\n", nGrabSpeed );
      cleanup_and_exit( camera );
   }

   int bytesPerPacket = nGrabSpeed * BYTES_PER_PACKET_200_FPS / 100;

   unsigned int 	nCols = 640;
   unsigned int 	nRows = 480;
   err = dc1394_format7_set_roi( camera,
				 DC1394_VIDEO_MODE_FORMAT7_0,
				 DC1394_COLOR_CODING_MONO8,
				 bytesPerPacket, // bytes per packet
				 4, 2,
				 nCols, nRows );

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

   printf( "Capturing %d images for profiling\n", nImagesToGrab );

   struct timeval start, end;
   gettimeofday( &start, NULL );

   for ( int i = 0; i <  nImagesToGrab; i++ )
   {
      printf( "\rCapturing image %d...", i );
      fflush( stdout );
      //  capture one frame
      if ( dc1394_capture_dma( &camera, 1, DC1394_VIDEO1394_WAIT )!= DC1394_SUCCESS) 
      {
	 fprintf( stderr, "Unable to capture a frame\n" );
	 cleanup_and_exit( camera );
      }
      
      if ( i != nImagesToGrab-1 )
      {
	 // not last call so free buffer
	 dc1394_capture_dma_done_with_buffer( camera );
      }
   }

   printf( "\n" );

   gettimeofday( &end, NULL );
   int endms = end.tv_sec*1000 + end.tv_usec/1000;
   int startms = start.tv_sec*1000 + start.tv_usec/1000;
   int ms = endms - startms;
   double fps = nImagesToGrab*1000.0/(double)ms;
   
   printf( "Grabbed %d frames in %d milliseconds or %f fps\n", nImagesToGrab, ms, fps );

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
   
   fprintf( imagefile, "P5\n%u %u 255\n", nCols, nRows );
   fwrite( (const char *)dc1394_capture_get_dma_buffer (camera), 1,
	   nCols*nRows, imagefile );
   fclose( imagefile );
   printf( "wrote: grey.pgm\n" );
   
   // close camera
   cleanup_and_exit( camera );
   
   return 0;
}
