/**************************************************************************
 * 
 * Title:	grabdma
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
#include <libraw1394/raw1394.h>
#include <libdc1394/dc1394_control.h>
#include <stdlib.h>
#include <sys/time.h>


/*===== User specific configuration ====*/

/* You will need to modify the configuration below to match your camera and setup */

/* Define one of these descriptors.
 * More formats and modes are available, but these "most common" configurations
 * examples are provided.
 *
 * (Note: this example was initially written for Point Grey Research cameras.
 * So "most common" means "most common with PGR cameras".
 * Typical settings for Dragonfly, Flea or Scorpion cameras would be 8 bit mode.
 * Bumblebee cameras transmit in 16 bit mode.)
 *
 */

// #define CAMERA_1024x768_8BIT
// #define CAMERA_1024x768_16BIT
#define CAMERA_640x480_8BIT
// #define CAMERA_640x480_16BIT

#ifdef CAMERA_1024x768_8BIT
   int format 		= FORMAT_SVGA_NONCOMPRESSED_1;
   int mode		= MODE_1024x768_MONO;
   int frameRate	= FRAMERATE_15;
   int bytesPerPixel	= 1;
#endif

#ifdef CAMERA_1024x768_16BIT
   int format 		= FORMAT_SVGA_NONCOMPRESSED_1;
   int mode		= MODE_1024x768_MONO16;
   int frameRate	= FRAMERATE_15;
   int bytesPerPixel	= 2;
#endif

#ifdef CAMERA_640x480_8BIT
   int format 		= FORMAT_VGA_NONCOMPRESSED;
   int mode		= MODE_640x480_MONO;
   int frameRate	= FRAMERATE_30;
   int bytesPerPixel	= 1;
#endif

#ifdef CAMERA_640x480_16BIT
   int format 		= FORMAT_VGA_NONCOMPRESSED;
   int mode		= MODE_640x480_MONO16;
   int frameRate	= FRAMERATE_30;
   int bytesPerPixel	= 2;
#endif

/* 
 * Define one of the cards below for use.  If your computer has only one 1394 interface
 * card you should be fine with CARD_0.  If not, run testlibraw and see which of your
 * interfaces has your camera on it, then define that one.
 */

#define USE_CARD_0
//#define USE_CARD_1
//#define USE_CARD_2
//#define USE_CARD_3

#ifdef USE_CARD_0
   int nCard		= 0;
   const char* device	= "/dev/video1394/0";
#endif
#ifdef USE_CARD_1
   int nCard		= 1;
   const char* device	= "/dev/video1394/1";
#endif
#ifdef USE_CARD_2
   int nCard		= 2;
   const char* device	= "/dev/video1394/2";
#endif
#ifdef USE_CARD_3
   int nCard		= 3;
   const char* device	= "/dev/video1394/3";
#endif


/*===== Global Variables ====*/

raw1394handle_t 	raw1394Handle;
dc1394_cameracapture 	dc1394Camera;

bool			bRaw1394HandleCreated = false;
bool			bDc1394CameraCreated = false;

const char*		saveFileName = "image.pgm";





/*==== cleanup()
 * This is called when the program exits and destroys the existing connections to the
 * 1394 drivers
 *====*/
void 
cleanup( void ) 
{
   if ( bDc1394CameraCreated )
   {
      dc1394_dma_unlisten( raw1394Handle, &dc1394Camera );
      dc1394_dma_release_camera( raw1394Handle, &dc1394Camera );
   }

   if ( bRaw1394HandleCreated )
   {
      raw1394_destroy_handle( raw1394Handle );
   }
}

int main(int argc, char *argv[]) 
{
   FILE* 	imagefile;

   /*-----------------------------------------------------------------------
    * Open ohci and assign handle
    * Note: the variable specifies which 1394 card to open.  If you have multiple cards
    * you may want to try changing the USE_CARD definitions above
    *-----------------------------------------------------------------------*/
   raw1394Handle = dc1394_create_handle( nCard );
   if ( raw1394Handle == NULL )
   {
      fprintf( stderr, "Unable to aquire a raw1394 handle\n\n"
	       "Please check \n"
	       "  - if the kernel modules `ieee1394',`raw1394' and `ohci1394' are loaded\n"
	       "  - if you have read/write access to /dev/raw1394\n" );
      return 1;
   }
   bRaw1394HandleCreated = true;
   
   
   /*-----------------------------------------------------------------------
    *  get the camera nodes and describe them as we find them
    *-----------------------------------------------------------------------*/
   int numNodes;
   numNodes = raw1394_get_nodecount( raw1394Handle );

   nodeid_t * 	cameraNodes;
   int		numCameras;
   /* Note: set 3rd parameter to 0 if you do not want the camera details printed */
   cameraNodes = dc1394_get_camera_nodes( raw1394Handle, &numCameras, 1 );
   fflush( stdout );
   if ( numCameras < 1 )
   {
      /* if you get this message and you have multiple 1394 cards you may want to try
       * modifying the input parameter for dc1394_create_handle() above
       */
      fprintf( stderr, 
	       "No cameras found! (%d nodes on the bus)\n"
	       "  - could be you need to try a different 1394 device (modify code to fix)\n", 
	       numNodes );
      cleanup();
      return 1;
   }
   printf("Working with the first camera on the bus\n");
   
   /*-----------------------------------------------------------------------
    *  to prevent the iso-transfer bug from raw1394 system, check if
    *  camera is highest node. For details see 
    *  http://linux1394.sourceforge.net/faq.html#DCbusmgmt
    *  and
    *  http://sourceforge.net/tracker/index.php?func=detail&aid=435107&group_id=8157&atid=108157
    *-----------------------------------------------------------------------*/
   if( cameraNodes[0] == numNodes-1)
   {
      fprintf( stderr, "\n"
	       "Sorry, your camera is the highest numbered node\n"
	       "of the bus, and has therefore become the root node.\n"
	       "The root node is responsible for maintaining \n"
	       "the timing of isochronous transactions on the IEEE \n"
	       "1394 bus.  However, if the root node is not cycle master \n"
	       "capable (it doesn't have to be), then isochronous \n"
	       "transactions will not work.  The host controller card is \n"
	       "cycle master capable, however, most cameras are not.\n"
	       "\n"
	       "The quick solution is to add the parameter \n"
	       "attempt_root=1 when loading the OHCI driver as a \n"
	       "module.  So please do (as root):\n"
	       "\n"
	       "   rmmod ohci1394\n"
	       "   insmod ohci1394 attempt_root=1\n"
	       "\n"
	       "for more information see the FAQ at \n"
	       "http://linux1394.sourceforge.net/faq.html#DCbusmgmt\n"
	       "\n");
      cleanup();
      return 1;
   }
   
   /*-----------------------------------------------------------------------
    *  setup capture
    *-----------------------------------------------------------------------*/
   unsigned int channel;
   unsigned int speed;
   if ( dc1394_get_iso_channel_and_speed( raw1394Handle, 
					  cameraNodes[0],
					  &channel, 
					  &speed ) !=DC1394_SUCCESS )  
   {
      fprintf( stderr, "Unable to get the iso channel number\n" );
      cleanup();
      return 1;
   }



   // note: format, mode, frameRate and bytesPerPixel are all defined as globals 
   // in the header

   /* Setup the capture mode */
   int e = dc1394_dma_setup_capture( raw1394Handle, 
				     cameraNodes[0],
				     channel,		
				     format,
				     mode,
				     SPEED_400, 
				     frameRate,
				     8,	// number of buffers
				     1,	// drop frames
				     device,
				     &dc1394Camera );
   if ( e != DC1394_SUCCESS )
   {
      fprintf( stderr,"Unable to setup camera-\n"
	       "check code above line %d of %s to make sure\n"
	       "that the video mode,framerate and format are\n"
	       "supported by your camera\n",
	       __LINE__,__FILE__);
      cleanup();
      return 1;
   }
   bDc1394CameraCreated = true;
   

   /*-----------------------------------------------------------------------
    *  have the camera start sending us data
    *-----------------------------------------------------------------------*/
   printf( "Starting iso transmission...\n" );
   if ( dc1394_start_iso_transmission( raw1394Handle, dc1394Camera.node )
	!=DC1394_SUCCESS ) 
   {
      fprintf( stderr, "Unable to start camera iso transmission\n" );
      cleanup();
      return 1;
   }

   printf( "Capturing 30 images for profiling\n" );

   struct timeval start, end;
   gettimeofday( &start, NULL );

   for ( int i = 0; i < 30; i++ )
   {
      printf( "Capturing image %d...\n", i );
      if ( dc1394_dma_single_capture( &dc1394Camera ) != DC1394_SUCCESS ) 
      {
	 printf( "capture %d failed\n", i );
      }
      else 
      {
	 dc1394_dma_done_with_buffer( &dc1394Camera );
      }
   }

   gettimeofday( &end, NULL );
   int endms = end.tv_sec*1000 + end.tv_usec/1000;
   int startms = start.tv_sec*1000 + start.tv_usec/1000;
   int ms = endms - startms;
   double fps = 30.0*1000.0/(double)ms;
   
   printf( "Grabbed 30 frames in %d milliseconds or %f fps\n", ms, fps );

   
   /*-----------------------------------------------------------------------
    *  capture one more frame to save to file
    *-----------------------------------------------------------------------*/
   printf( "Capturing one image to save to file...\n" );
   if (dc1394_dma_single_capture( &dc1394Camera )!=DC1394_SUCCESS) 
   {
      fprintf( stderr, "Unable to capture a frame\n");
      cleanup();
      return 1;
   }
   
   /*-----------------------------------------------------------------------
    *  save image 
    *-----------------------------------------------------------------------*/
   printf( "Saving the image...\n" );
   imagefile = fopen( saveFileName, "w" );
   
   if( imagefile == NULL)
   {
      perror("");
      fprintf( stderr, "Can't create '%s'\n", saveFileName );
      cleanup();
      return 1;
   }
   
   fprintf( imagefile,
	    "P5\n%u %u 255\n", 
	    dc1394Camera.frame_width,
	    dc1394Camera.frame_height );
   if ( bytesPerPixel == 1 )
   {
      fwrite( (const char *)dc1394Camera.capture_buffer, 1,
	      dc1394Camera.frame_height*dc1394Camera.frame_width, imagefile );
   }
   else
   {
      // write only the first byte of each pixel
      unsigned char* p = (unsigned char*) dc1394Camera.capture_buffer;
      for ( int i = 0; i < dc1394Camera.frame_height*dc1394Camera.frame_width; i++ )
      {
	 fwrite( p, 1, 1, imagefile );
	 p += bytesPerPixel;
      }
   }
   fclose( imagefile );
   printf( "saved image to '%s'\n", saveFileName );

   dc1394_dma_done_with_buffer( &dc1394Camera );

   /*-----------------------------------------------------------------------
    *  Stop data transmission
    *-----------------------------------------------------------------------*/
   if ( dc1394_stop_iso_transmission( raw1394Handle,dc1394Camera.node ) != DC1394_SUCCESS ) 
   {
      printf("Couldn't stop the camera?\n");
   }
   
   
   /*-----------------------------------------------------------------------
    *  Close camera
    *-----------------------------------------------------------------------*/
   cleanup();
   return 0;
}


/*************************************************************************
 *
 * $RCSfile: grabdma.cpp,v $
 * $Revision: 1.4 $
 * $Date: 2005/04/07 21:05:14 $
 * $Log: grabdma.cpp,v $
 * Revision 1.4  2005/04/07 21:05:14  donm
 * [1] fixed a bug in grabdma - it was using FRAMERATE_120 which was wrong
 * [2] added a user-configuration section that includes specifying which
 *     interface card the user is using
 *
 * Revision 1.3  2005/03/15 19:32:41  donm
 * updating the LGPL notice etc
 *
 *
 *************************************************************************/
