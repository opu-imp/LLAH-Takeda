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
// 結合画像生成
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
	// 画像サイズ取得
	img_size = src.size();
	// 画像バッファ
	bin = cv::Mat::zeros( img_size, CV_8UC1 );
	inv = cv::Mat::zeros( img_size, CV_8UC1 );
	smt = cv::Mat::zeros( img_size, CV_8UC1 );

	if ( eEntireMode == RETRIEVE_MODE || eEntireMode == MAKE_QUERY_DAT_MODE ) {
		// 適応二値化（1回目）
		cv::adaptiveThreshold( src, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, eBlockSize, eC );
		cv::bitwise_not( bin, inv );

		// 輪郭抽出（ノイズ除去のため）
		cv::findContours( inv, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );

		// 輪郭描画（ノイズ除去のため）
		for ( size_t ci = 0; ci < contours.size(); ci++ ) {
			if ( hierarchy[ci][3] != -1 )	continue;	// 最外輪郭のみ（旧 C API の h_next 巡回と同じ）
			mom = cv::moments( contours[ci] );
			d00 = mom.m00;
			if ( d00 < 10 )	continue;
			cv::drawContours( smt, contours, (int)ci, cWhite, cv::FILLED, 8 );
		}
		cv::bitwise_not( smt, bin );

		// 平滑化
		cv::GaussianBlur( bin, smt, cv::Size(eMask, eMask), 0, 0, cv::BORDER_REPLICATE );

		// 適応二値化（2回目）
		cv::adaptiveThreshold( smt, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, eBlockSize, eC );
		cv::bitwise_not( bin, inv );
	} else {	// ハッシュ構築モード（電子文書）：閾値固定モード
		cv::GaussianBlur( src, smt, cv::Size(15, 15), 0, 0, cv::BORDER_REPLICATE );
		cv::adaptiveThreshold( smt, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, eBlockSize, eC );
		cv::bitwise_not( bin, inv );
	}

	return inv;
}

cv::Mat GetConnectedImageForSmartphone( cv::Mat img )
// 結合画像生成
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

	// 適応二値化（1回目）
	cv::adaptiveThreshold( img, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 101, 10 );
	cv::bitwise_not( bin, inv );

	// 輪郭抽出（ノイズ除去のため）
	cv::findContours( inv, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );

	// 輪郭描画（ノイズ除去のため）
	for ( size_t ci = 0; ci < contours.size(); ci++ ) {
		if ( hierarchy[ci][3] != -1 )	continue;	// 最外輪郭のみ（旧 C API の h_next 巡回と同じ）
		mom = cv::moments( contours[ci] );
		d00 = mom.m00;
		if ( d00 < 10 )	continue;
		cv::drawContours( smt, contours, (int)ci, cWhite, cv::FILLED, 8 );
	}
	cv::bitwise_not( smt, bin );
	cv::bitwise_not( bin, inv );

	// 文字の大きさを調べる
	contours.clear();
	hierarchy.clear();
	cv::findContours( inv, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );

	// 連結成分を描画・重心を計算
	InitHist( &hist, 1000, 0, 1000 );
	for( size_t ci = 0; ci < contours.size(); ci++ )
	{
		if ( hierarchy[ci][3] != -1 )	continue;	// 最外輪郭のみ（旧 C API の h_next 巡回と同じ）
		mom = cv::moments( contours[ci] );
		d00 = mom.m00;
		if ( d00 <= 0.5 ) continue;
		Add2Hist( &hist, sqrt(d00) );
	}

	chsize = (int)GetMaxBin(&hist);
	ReleaseHist( &hist );
	chsize = chsize * 2 + 1;

	// 平滑化
	cv::GaussianBlur( bin, smt, cv::Size(1, 1), 0, 0, cv::BORDER_REPLICATE );

	// 適応二値化（2回目）
	cv::adaptiveThreshold( smt, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 101, 10 );
	cv::bitwise_not( bin, inv );

	return inv;
}

int MakeFeaturePoint( cv::Mat img, cv::Point **ps0, double **areas0, cv::Size *size0 )
	// 特徴点抽出・連結成分の面積計算
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

	// 画像サイズの取得
	*size0 = img.size();
#ifdef DRAW_FP
	fp = cv::Mat::zeros( *size0, CV_8UC3 );
#endif

	// 輪郭抽出
	cv::findContours( img, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );

	// 特徴点候補数のカウント（最外輪郭のみ．旧 C API の h_next 巡回と同じ）
	i = 0;
	for ( size_t ci = 0; ci < contours.size(); ci++ )	if ( hierarchy[ci][3] == -1 )	i++;
	num = ( i >= kMaxPointNum ) ? kMaxPointNum - 1 : i;
	if ( num <= 0 )	return 0;

	// 配列の確保
	ps = (cv::Point *)calloc( num, sizeof(cv::Point) );
	areas = (double *)calloc( num, sizeof(double) );

	i = 0;
	// 特徴点抽出・面積計算
	for ( size_t ci = 0; ci < contours.size() && i < num ; ci++ ) {
		if ( hierarchy[ci][3] != -1 )	continue;	// 最外輪郭のみ（旧 C API の h_next 巡回と同じ．穴は特徴点にしない）
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
	// 正確な特徴点数に更新
	num = i;

	// ポインタ渡し
	*ps0 = ps;
	*areas0 = areas;

	return num;
}
