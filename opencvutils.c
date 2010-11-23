#include "opencvutils.h"

IplImage *dc1394_frame_get_iplimage(dc1394video_frame_t *frame)
{
    g_return_val_if_fail(frame != NULL, NULL);
    g_return_val_if_fail(frame->padding_bytes == 0, NULL);

    IplImage *img;
    dc1394video_mode_t video_mode = frame->video_mode;
    CvSize size = cvSize(frame->size[0], frame->size[1]);

    if (video_mode == DC1394_VIDEO_MODE_640x480_MONO8) {
        img = cvCreateImageHeader(size, IPL_DEPTH_8U, 1);

        /* memcpy returns a pointer to dest
           FIXME: USE cvSetData(tmp, imdata, m_frame->size[0]*3); */
        img->imageDataOrigin = memcpy(
                                &(img->imageData),
                                frame->image,
                                frame->size[0]*frame->size[1]*1*sizeof(unsigned char));
    } else if ((video_mode == DC1394_VIDEO_MODE_FORMAT7_0) ||
               (video_mode == DC1394_VIDEO_MODE_FORMAT7_1)) {
            dc1394error_t err;
            dc1394video_frame_t dest;
            IplImage *tmp;
            unsigned char *imdata;

            img = cvCreateImageHeader(size, IPL_DEPTH_8U, 3);

            /* debayer frame into RGB8 */
            imdata = (unsigned char *)malloc(frame->size[0]*frame->size[1]*3*sizeof(unsigned char));
            dest.image = imdata;
            dest.color_coding = DC1394_COLOR_CODING_RGB8;
            err=dc1394_debayer_frames(frame, &dest, DC1394_BAYER_METHOD_NEAREST); 
            if (err != DC1394_SUCCESS)
                dc1394_log_error("Could not convert/debayer frames");

            /* convert from RGB to BGR */
            tmp = cvCreateImageHeader(cvSize(frame->size[0], frame->size[1]), IPL_DEPTH_8U, 3);
            cvSetData(tmp, imdata, frame->size[0]*3);

            cvCvtColor(tmp, img, CV_RGB2BGR);

            free(imdata);
            cvReleaseImageHeader(&tmp);
    } else {
        g_assert_not_reached();
    }

    return img;
}

IplImage *dc1394_capture_get_iplimage(dc1394camera_t *camera)
{
    dc1394error_t err;
    dc1394video_frame_t *frame;
    IplImage *img;

    err = dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame);
    DC1394_WRN(err,"Could not capture a frame");

    img = dc1394_frame_get_iplimage(frame);

    err = dc1394_capture_enqueue(camera, frame);
    DC1394_WRN(err,"releasing buffer");

    return img;
}
