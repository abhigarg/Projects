#include "stdafx.h"
//#include "boundingBoxDetect.h"

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


int main(void)//(int argc, char** argv)
{
	// start recording time
	clock_t t = clock();
	
	// input image
	IplImage* input_img = cvLoadImage("saveimg99.jpg");

	// check if input image is valid or not
	if(!input_img->imageData)
	{
		printf("input image not valid \n");
		return -10;
	}

	// display input image
	cvShowImage("input image", input_img);
	cvWaitKey(0);

	IplImage* new_img = cvCreateImage(cvGetSize(input_img), input_img->depth, input_img->nChannels);
	cvCopy(input_img, new_img);

	// make background white color
	cvFloodFill(new_img, cvPoint(0,0), cvScalar(255,255,255), cvScalarAll(8), cvScalarAll(255), NULL, 8, NULL);
	//cvShowImage("White Background Image", input_img);
	//cvWaitKey(0);

	cvShowImage("white background image", new_img);
	cvWaitKey(0);
	
	IplImage* in_img = cvCreateImage(cvGetSize(input_img),input_img->depth,3);
	
	// smoothen image
	cvSmooth(new_img, in_img, CV_BLUR, 11, 11);

	cvShowImage("smooth image", new_img);
	cvWaitKey(0);

	// select range of pixels to identify objects
	IplImage* range_img = cvCreateImage(cvSize(input_img->width, input_img->height), 8, 1);
	cvInRangeS(in_img, CV_RGB(252,252,252), CV_RGB(255, 255, 255), range_img); 	

	cvShowImage("range image", range_img);
	cvWaitKey(0);

	// Apply canny edge detection
	IplImage* canny_img = cvCreateImage(cvGetSize(range_img), 8, 1);
	cvCanny(range_img, canny_img, 2, 8, 3);

	cvShowImage("canny edge", canny_img);
	cvWaitKey(0);

	// identify possible object edges
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* pts;
    int i;

	// contours
	CvSeq* contours1 = 0;
	CvSeq* contours = NULL;
	cvFindContours(canny_img, storage, &contours1, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, cvPoint(0,0));
	
	contours = cvApproxPoly(contours1, sizeof(CvContour), storage, CV_POLY_APPROX_DP, 2, 1);

	int cnt = 0;
	CvBox2D rect;
	CvPoint2D32f corner[4];
	int area = 0;
	CvPoint rect_pts[4], *pt = rect_pts;
	int npts = 4;
	double dist;

	vector<CvPoint2D32f> boundaryPt, refPt, objPt;
	vector<CvSeq*> edgeSeq;
	vector<int> crossFlag;
	int flagStatus;
	CvSeqReader reader;

	CvPoint2D32f pt_topleft, pt_topright, pt_bottomleft, pt_bottomright, temp;

	pt_topleft.x = 89;
	pt_topleft.y = 33;

	pt_topright.x = 656;
	pt_topright.y = 35;

	pt_bottomleft.x = 84;
	pt_bottomleft.y = 277;

	pt_bottomright.x = 654;
	pt_bottomright.y = 275;

	boundaryPt.push_back(pt_topleft);
	
	float x = pt_topleft.x;
	float m1 = (pt_topright.y-pt_topleft.y)/(pt_topright.x-pt_topleft.x);
	float y;

	while(x < pt_topright.x)
	{
		x = x + 0.1;
		y = m1*(x - pt_topleft.x) + pt_topleft.y;
		temp.x = x;
		temp.y = y;
		boundaryPt.push_back(temp);
	}

	y = pt_topright.y;
	float m2 = (pt_bottomright.y-pt_topright.y)/(pt_bottomright.x-pt_topright.x);

	while(y < pt_bottomright.y)
	{
		y = y + 0.1;
		x = pt_topright.x + (y - pt_topright.y)/m2;
		temp.x = x;
		temp.y = y;
		boundaryPt.push_back(temp);
	}
	
	x = pt_bottomright.x;
	float m3 = (pt_bottomleft.y-pt_bottomright.y)/(pt_bottomleft.x-pt_bottomright.x);

	while(x > pt_bottomleft.x)
	{
		x = x - 0.1;
		y = m3*(x - pt_bottomright.x) + pt_bottomright.y;
		temp.x = x;
		temp.y = y;
		boundaryPt.push_back(temp);
	}

	y = pt_bottomleft.y;
	float m4 = (pt_topleft.y-pt_bottomleft.y)/(pt_topleft.x-pt_bottomleft.x);

	while(y > pt_topleft.y)
	{
		y = y - 0.1;
		x = pt_bottomleft.x + (y - pt_bottomleft.y)/m4;
		temp.x = x;
		temp.y = y;
		boundaryPt.push_back(temp);
	}

	CvPoint tempPt, p;
	
	// boundary lines
	cvLine(input_img, cvPointFrom32f(pt_topleft), cvPointFrom32f(pt_topright), CV_RGB(0,0,0),2);
	cvLine(input_img, cvPointFrom32f(pt_topright), cvPointFrom32f(pt_bottomright), CV_RGB(0,0,0),2);
	cvLine(input_img, cvPointFrom32f(pt_bottomright), cvPointFrom32f(pt_bottomleft), CV_RGB(0,0,0),2);
	cvLine(input_img, cvPointFrom32f(pt_bottomleft), cvPointFrom32f(pt_topleft), CV_RGB(0,0,0),2);

	// draw contours bounding box
	for (; contours != 0; contours = contours->h_next){
		cnt++;
		
		rect = cvMinAreaRect2 (contours, 0); //extract bounding box for current contour
		area = (int)(rect.size.height*rect.size.width);

		//bounding box rectangle
		cvBoxPoints(rect, corner);
		for(int i=0;i<4;i++)
		{
			rect_pts[i] = cvPointFrom32f(corner[i]);
		}

		if(rect.size.height<20 && rect.size.width<20 && ((rect.center.x < input_img->width/5) || rect.center.x > (4*input_img->width/5)) && 
			(rect.center.y < input_img->height/5 || rect.center.y > (4*input_img->height/5)))
		{
			cvDrawContours(input_img, contours, CV_RGB( 255, 0, 255 ), CV_RGB(0,0,0), -1, 3, 8, cvPoint(0,0)); // contour for ellipses
			refPt.push_back(rect.center);
		}

		if(area>1000 && area<15000)
		{ 
			//cvDrawContours(input_img, contours, CV_RGB( 0, 0, 255 ), CV_RGB(0,0,0), -1, 3, 8, cvPoint(0,0)); // contour
			// draw bounding rectangle
			for( int j = 0; j < 4; j++ )
				cvLine( input_img, rect_pts[j], rect_pts[(j+1)%4], CV_RGB( 0, 0, 255 ), 2, 8 );
			objPt.push_back(rect.center);
			
			edgeSeq.push_back(contours);

			//cvEllipseBox(in_img, rect, CV_RGB(255,0,0),2,8,0);
			//cvPolyLine(in_img, &pt, &npts, 1, 1, CV_RGB(0,255,0), 2, 8, 0);
			//printf("object found at (%f x %f)\n", rect.center.x, rect.center.y);
			for(int i = 0; i < boundaryPt.size(); i++)
			{
				//polyContours = cvApproxPoly(contours, sizeof(CvContour), storage, CV_POLY_APPROX_DP, 2, 1);
				dist = cvPointPolygonTest(contours, boundaryPt[i], 0);
				
				flagStatus = 0;  // boundary is not crossed

				//cvCircle(input_img, cvPointFrom32f(boundaryPt[i]), 1, CV_RGB(24, 160, 12));
				if(dist>=0)	
				{
					flagStatus = 1;
					//crossFlag.push_back(1); // flag for object crossed the boundary
					printf("object crosses boundary\n");

					cvDrawContours(input_img, contours, CV_RGB( 255, 0, 0 ), CV_RGB(0,0,0), -1, 3, 8, cvPoint(0,0)); // contour
					for( int j = 0; j < 4; j++ )
						cvLine( input_img, rect_pts[j], rect_pts[(j+1)%4], CV_RGB( 255, 0, 0 ), 2, 8 );
					
					break;
				}
			}
			crossFlag.push_back(flagStatus);

			cvDrawContours(input_img, contours, CV_RGB( 255, 0, 255 ), CV_RGB(0,0,0), -1, 3, 8, cvPoint(0,0)); // contour for objects detected
			//cvDrawContours(input_img, polyContours, CV_RGB( 25, 255, 55 ), CV_RGB(0,0,0), -1, 3, 8, cvPoint(5,5)); // contour for objects detected

		}
	}
	t = clock() - t;
	float tot_time = (float) t/CLOCKS_PER_SEC;
	printf("total time taken in processing: %f\n", tot_time);
	

	// write to xml file
	FileStorage fs;
	fs.open("output.xml", FileStorage::WRITE);
	
	CvSeq* contour = 0;
	if(fs.isOpened())
	{
		fs << "PANE" << "{";
		for(int i=0; i < refPt.size(); i++)
		{
			fs << "REFERENCE" << refPt[i].x;
			fs << "REFERENCE" << refPt[i].y;
		}
		fs << "}";
		fs << "OBJECT" << "{";
		for(int i=0; i<objPt.size(); i++)
		{
			fs << "CROSSED" << crossFlag[i];
			fs << "CENTRE_X" << objPt[i].x; 
			fs << "CENTRE_Y" << objPt[i].y;
			fs << "POINTS" << "{";
			
			for(; edgeSeq[i]!=NULL;edgeSeq[i]=edgeSeq[i]->h_next)
			{
				cvStartReadSeq(edgeSeq[i], &reader);
				for(int j=0;j<edgeSeq[i]->total; j++)
				{
					CV_READ_SEQ_ELEM(p, reader);
					//printf("point x, point y %d,%d\n", p.x, p.y);
					fs << "EDGE_X" << p.x; 
					fs << "EDGE_Y" << p.y;
				}
			}
			fs << "}";
		}
		fs << "}";
	}
    fs.release();
	cvShowImage("output", input_img);
	cvSaveImage("output.jpg", input_img);
	cvWaitKey(0);
	return 0;
}

