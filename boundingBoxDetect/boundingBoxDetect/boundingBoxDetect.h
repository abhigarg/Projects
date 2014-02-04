# ifndef _BOUNDINGBOXDETECT_H_
# define _BOUNDINGBOXDETECT_H_

#include "cv.h"
#include "highgui.h"
#include <cvaux.h>
#include <cxcore.h>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <limits>
#include <fstream>
#include <math.h>
#include <stdlib.h>
#include <string.h>


using namespace std;
using namespace cv;


vector<int> findBackgroundColor(IplImage* input_img)
{
	int hist_size = 256;
    float range[] = {0, 256};
    float *ranges[] = {range};

	IplImage* blue_img = cvCreateImage( cvGetSize(input_img), IPL_DEPTH_8U, 1 );
	IplImage* green_img = cvCreateImage( cvGetSize(input_img), IPL_DEPTH_8U, 1 );
    IplImage* red_img = cvCreateImage( cvGetSize(input_img), IPL_DEPTH_8U, 1 );
    cvCvtPixToPlane( input_img, blue_img, green_img, red_img, 0 );

	CvHistogram* hist_red = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
    CvHistogram* hist_blue = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
    CvHistogram* hist_green = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);

	cvCalcHist( &red_img, hist_red, 0, NULL );
    cvCalcHist( &blue_img, hist_blue, 0, NULL );
    cvCalcHist( &green_img, hist_green, 0, NULL );

	float min_val_red = 0;
	float max_val_red = 0;
	int min_id_red = 0;
	int max_id_red = 0;
	cvGetMinMaxHistValue(hist_red, &min_val_red, &max_val_red, &min_id_red, &max_id_red);

	float min_val_green = 0;
	float max_val_green = 0;
	int min_id_green = 0;
	int max_id_green = 0;
	cvGetMinMaxHistValue(hist_green, &min_val_green, &max_val_green, &min_id_green, &max_id_green);

	float min_val_blue = 0;
	float max_val_blue = 0;
	int min_id_blue = 0;
	int max_id_blue = 0;
	cvGetMinMaxHistValue(hist_blue, &min_val_blue, &max_val_blue, &min_id_blue, &max_id_blue);

	printf("value of background:%d, %d, %d\n", max_id_red, max_id_green, max_id_blue);

	vector<int> backgroundColor(3);
	backgroundColor[0] = max_id_red;
	backgroundColor[1] = max_id_green;
	backgroundColor[2] = max_id_blue;

	return backgroundColor;
}


# endif 

