#include <stdio.h>
#include "def_general.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "procimg.h"
#include "outputimg.h"


int OutPutImage(cv::Mat img)
{
	static int n = 0;
	char filename[1024];

	sprintf(filename, "output%03d.jpg", n++);

	cv::imwrite( filename, img );
	return 0;
}

int SetPixU( cv::Mat &img, int x, int y, unsigned char *val )
{
	int i, bpp;

	if ( x < 0 || x >= img.cols || y < 0 || y >= img.rows )	return 0;
	bpp = img.elemSize();
	for ( i = 0; i < bpp; i++ )
		img.data[img.step[0]*y + x*bpp + i] = val[i];
	return 1;
}

int GetPixU( cv::Mat &img, int x, int y, unsigned char *val )
{
	int i, bpp;

	if ( x < 0 || x >= img.cols || y < 0 || y >= img.rows )	return 0;
	bpp = img.elemSize();
	for ( i = 0; i < bpp; i++ )
		val[i] = img.data[img.step[0]*y + x*bpp + i];
	return 1;
}

cv::Mat MakeFeaturePointImage( cv::Mat orig, cv::Mat connected, cv::Point *ps, int num )
{
	int i, j, p, con_flag = 0;
	cv::Mat fp_img, fp;
	unsigned char val[4];
	cv::bitwise_not( connected, connected );
	fp_img = cv::Mat(orig.size(), CV_8UC3);
	fp = cv::Mat(orig.size(), CV_8UC3);
	fp_img.setTo(0);
	cv::bitwise_not( fp_img, fp_img );
	if ( fp_img.empty() )	return cv::Mat();
	for ( j = 0; j < orig.rows; j++ ) {
		for ( i = 0; i < orig.cols; i++ ) {
			con_flag = 0;
			GetPixU( connected, i, j, val );
			if ( val[0] != 255 ) {
				val[0] = 0;
				val[1] = 255;
				val[2] = 0;
				SetPixU( fp_img, i, j, val );
				con_flag = 1;
			}
			GetPixU( orig, i, j, val );
			if ( val[0] != 255 ) {
				if ( con_flag ) {
					val[1] = val[0];
					val[0] = 0;
					val[2] = 0;
					SetPixU( fp_img, i, j, val );
				}
				else {
					val[1] = val[0];
					val[2] = val[0];
					SetPixU( fp_img, i, j, val );
				}
			}
		}
	}

	for ( p = 0; p < num; p++ ) {
		cv::circle( fp_img, ps[p], 4, cRed, cv::FILLED, cv::LINE_AA, 0 );
	}
	cv::bitwise_not( fp, fp );

	return fp_img;
}

#define	kDrawCorHMargin	(100)
#define	kDrawCorVMargin	(100)
#define	kDrawCorSpace	(400)
#define	kDrawCorScale	(0.5)
#define	kDrawCorRectThick	(4)
#define	kDrawCorPtRad	(4)
#define	kDrawCorLineThick	(4)

void DrawCor( cv::Point *ps, int num, cv::Size img_size, int res, cv::Point *corps, int cornum, cv::Size corsize, int pcor[][2], int pcornum )
{
	int i, width_all, height_all;

	img_size.height = 4368;
	img_size.width = 2912;
	corsize.height = 2200;
	corsize.width = 1700;
	
	width_all = kDrawCorHMargin * 2 + kDrawCorSpace + img_size.width + (int)(corsize.width * kDrawCorScale);
	height_all = kDrawCorVMargin * 2 + std::max( img_size.height, (int)(corsize.height * kDrawCorScale) );
	cv::Mat img( cv::Size( width_all, height_all ), CV_8UC3 );
	img.setTo( cWhite );

	cv::rectangle( img, cv::Point( kDrawCorHMargin, kDrawCorVMargin ), \
		cv::Point( kDrawCorHMargin + img_size.width, kDrawCorVMargin + img_size.height ), \
		cBlack, kDrawCorRectThick, cv::LINE_AA, 0 );
	
	for ( i = 0; i < num; i++ ) {
		cv::circle( img, cv::Point( kDrawCorHMargin + ps[i].x, kDrawCorVMargin + ps[i].y ), kDrawCorPtRad, cBlack, -1, cv::LINE_AA, 0 );
	}
	cv::rectangle( img, cv::Point( kDrawCorHMargin + img_size.width + kDrawCorSpace, kDrawCorVMargin), \
		cv::Point( kDrawCorHMargin + img_size.width + kDrawCorSpace + (int)(corsize.width * kDrawCorScale), kDrawCorVMargin + (int)(corsize.height * kDrawCorScale) ), \
		cBlack, kDrawCorRectThick, cv::LINE_AA, 0 );
	for ( i = 0; i < cornum; i++ ) {
		cv::circle( img, cv::Point( kDrawCorHMargin + img_size.width + kDrawCorSpace + (int)(corps[i].x * kDrawCorScale), kDrawCorVMargin + (int)(corps[i].y * kDrawCorScale) ), \
		kDrawCorPtRad, cBlack, -1, cv::LINE_AA, 0 );
	}
	OutPutImage( img );
}
