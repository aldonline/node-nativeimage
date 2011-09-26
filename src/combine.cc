#include <opencv2/opencv.hpp>
#include "combine.h"

// http://opencv.willowgarage.com/wiki/Mac_OS_X_OpenCV_Port
// http://togaen.wordpress.com/2011/08/04/xcode-and-opencv-2-via-macports/

void combine4( char* path1, char* path2, char* path3, char* path4, char* dest_path ){
  // order in which they are combined:
  // 0 1
  // 2 3

  // load all images
  IplImage* img0 = cvLoadImage( path1 );
  IplImage* img1 = cvLoadImage( path2 );
  IplImage* img2 = cvLoadImage( path3 );
  IplImage* img3 = cvLoadImage( path4 );

  // if all images were added to a full-size montage
  // these would be the dimensions
  unsigned short w = img0->width + img1->width;
  unsigned short h = img0->height + img2->height;

  // create destination image, half the size of the full-size montage
  IplImage* img = cvCreateImage( cvSize(w/2, h/2), 8, 3 );

  // copy+resize each image onto destination
  IplROI roi0 = { 0, 0, 0, img0->width/2, img0->height/2 };
  img->roi = &roi0;
  cvResize( img0, img );

  IplROI roi1 = { 0, img0->width/2, 0, img1->width/2, img1->height/2 };
  img->roi = &roi1;
  cvResize( img1, img );

  IplROI roi2 = { 0, 0, img0->height/2, img2->width/2, img2->height/2 };
  img->roi = &roi2;
  cvResize( img2, img );

  IplROI roi3 = { 0, img0->width/2, img0->height/2, img3->width/2, img3->height/2 };
  img->roi = &roi3;
  cvResize( img3, img );

  // cleanup
  img->roi = NULL;
  cvReleaseImage(&img0);
  cvReleaseImage(&img1);
  cvReleaseImage(&img2);
  cvReleaseImage(&img3);

  // save result to disk
  cvSaveImage( dest_path, img );
}