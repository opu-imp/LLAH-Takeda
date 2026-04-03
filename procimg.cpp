#include "def_general.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#ifdef WIN32
#ifdef _DEBUG
#pragma comment( lib, "opencv_imgproc4d.lib" )
#pragma comment( lib, "opencv_core4d.lib" )
#pragma comment( lib, "opencv_highgui4d.lib" )
#else
#pragma comment( lib, "opencv_imgproc4.lib" )
#pragma comment( lib, "opencv_core4.lib" )
#pragma comment( lib, "opencv_highgui4.lib" )
#endif
#endif

#include "extern.h"
#include "outputimg.h"
#include "hist.h"

using namespace std;

cv::Mat GetConnectedImage( char *fname )
{
	int i, chsize;
	double d00, max_hist;
	cv::Mat src, bin, inv, smt;
	cv::Size img_size;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::Moments mom;
	strHist hist;

	src = cv::imread(fname, cv::IMREAD_GRAYSCALE);
	if ( src.empty() ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		return cv::Mat();
	}
	img_size = src.size();
	bin = cv::Mat::zeros( img_size, CV_8UC1 );
	inv = cv::Mat::zeros( img_size, CV_8UC1 );
	smt = cv::Mat::zeros( img_size, CV_8UC1 );

	if ( eEntireMode == RETRIEVE_MODE || eEntireMode == MAKE_QUERY_DAT_MODE ) {
		cv::adaptiveThreshold( src, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, eBlockSize, eC );
		cv::bitwise_not( bin, inv );

		cv::findContours( inv, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );

		for ( size_t ci = 0; ci < contours.size(); ci++ ) {
			mom = cv::moments( contours[ci] );
			d00 = mom.m00;
			if ( d00 < 10 )	continue;
			cv::drawContours( smt, contours, (int)ci, cWhite, cv::FILLED, 8 );
		}
		cv::bitwise_not( smt, bin );

		cv::GaussianBlur( bin, smt, cv::Size(eMask, 1), 0 );

		cv::adaptiveThreshold( smt, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, eBlockSize, eC );
		cv::bitwise_not( bin, inv );
	} else {
		cv::GaussianBlur( src, smt, cv::Size(15, 1), 0 );
		cv::adaptiveThreshold( smt, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, eBlockSize, eC );
		cv::bitwise_not( bin, inv );
	}

	return inv;
}

cv::Mat GetConnectedImageForSmartphone( cv::Mat img )
{
	double d00;
	cv::Mat bin, inv, smt;
	cv::Size img_size;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::Moments mom;
	strHist hist;
	int i=0, chsize=0;

	img_size = img.size();
	bin = cv::Mat::zeros( img_size, CV_8UC1 );
	inv = cv::Mat::zeros( img_size, CV_8UC1 );
	smt = cv::Mat::zeros( img_size, CV_8UC1 );

	cv::adaptiveThreshold( img, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 101, 10 );
	cv::bitwise_not( bin, inv );

	cv::findContours( inv, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );

	for ( size_t ci = 0; ci < contours.size(); ci++ ) {
		mom = cv::moments( contours[ci] );
		d00 = mom.m00;
		if ( d00 < 10 )	continue;
		cv::drawContours( smt, contours, (int)ci, cWhite, cv::FILLED, 8 );
	}
	cv::bitwise_not( smt, bin );
	cv::bitwise_not( bin, inv );

	// Find contours again for histogram
	contours.clear();
	hierarchy.clear();
	cv::findContours( inv, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );

	InitHist( &hist, 1000, 0, 1000 );
	for( size_t ci = 0; ci < contours.size(); ci++ )
	{
		mom = cv::moments( contours[ci] );
		d00 = mom.m00;
		if ( d00 <= 0.5 ) continue;
		Add2Hist( &hist, sqrt(d00) );
	}

	chsize = (int)GetMaxBin(&hist);
	ReleaseHist( &hist );
	chsize = chsize * 2 + 1;

	cv::GaussianBlur( bin, smt, cv::Size(1, 1), 0 );

	cv::adaptiveThreshold( smt, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 101, 10 );
	cv::bitwise_not( bin, inv );

	return inv;
}

int MakeFeaturePoint( cv::Mat img, cv::Point **ps0, double **areas0, cv::Size *size0 )
{
	int i, num;
	double d00;
	double *areas = NULL;
	cv::Point *ps;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::Moments mom;
#ifdef DRAW_FP
	cv::Mat fp;
#endif

	*size0 = img.size();
#ifdef DRAW_FP
	fp = cv::Mat::zeros( *size0, CV_8UC3 );
#endif

	cv::findContours( img, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );

	num = ( (int)contours.size() >= kMaxPointNum ) ? kMaxPointNum - 1 : (int)contours.size();
	if ( num <= 0 )	return 0;

	ps = (cv::Point *)calloc( num, sizeof(cv::Point) );
	areas = (double *)calloc( num, sizeof(double) );

	i = 0;
	for ( size_t ci = 0; ci < contours.size() && i < num ; ci++ ) {
		mom = cv::moments( contours[ci] );
		d00 = mom.m00;
		if ( d00 < 5 )	continue;
		ps[i].x = (int)(mom.m10 / d00 );
		ps[i].y = (int)(mom.m01 / d00 );
		areas[i] = d00;
#ifdef DRAW_FP
		cv::circle( fp, ps[i], 4, cWhite, -1, 1, 0 );
#endif
		i++;
	}

#ifdef DRAW_FP
	cv::bitwise_not( fp, fp );
	OutPutImage( fp );
#endif
	num = i;

	*ps0 = ps;
	*areas0 = areas;

	return num;
}
