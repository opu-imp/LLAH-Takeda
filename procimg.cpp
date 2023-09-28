#include "def_general.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/highgui/highgui_c.h>

#ifdef WIN32
#ifdef _DEBUG
#pragma comment( lib, "opencv_imgproc231d.lib" )
#pragma comment( lib, "opencv_core231d.lib" )
#pragma comment( lib, "opencv_highgui231d.lib" )
#else
#pragma comment( lib, "opencv_imgproc231.lib" )
#pragma comment( lib, "opencv_core231.lib" )
#pragma comment( lib, "opencv_highgui231.lib" )
#endif
#endif

#include "extern.h"
#include "outputimg.h"
#include "hist.h"

using namespace std;

IplImage *GetConnectedImage( char *fname )
// 結合画像生成
{
	int i, chsize;
	double d00, max_hist;
	IplImage *src, *bin, *inv, *smt;
	CvSize img_size;
	CvMemStorage *storage = cvCreateMemStorage(0);
	CvSeq *contours = 0, *con0;
	CvMoments mom;
	strHist hist;

	// 文書画像読み込み
	if ( (src = cvLoadImage(fname, 0)) == NULL ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		return NULL;
	}
	// 画像サイズ取得
	img_size = cvGetSize( src );
	// 画像バッファ
	bin = cvCreateImage( img_size, 8, 1 );
	inv = cvCreateImage( img_size, 8, 1 );
	smt = cvCreateImage( img_size, 8, 1 );
	cvZero( bin );
	cvZero( inv );
	cvZero( smt );

	if ( eEntireMode == RETRIEVE_MODE || eEntireMode == MAKE_QUERY_DAT_MODE ) {
		// 適応二値化（1回目）
		cvAdaptiveThreshold( src, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, eBlockSize, eC );
		cvNot( bin, inv );

		//OutPutImage( inv );

		// 輪郭抽出（ノイズ除去のため）
		cvFindContours( inv, storage, &contours, sizeof(CvContour),
			CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
	
		// 輪郭描画（ノイズ除去のため）
		for ( con0 = contours; con0 != 0; con0 = con0->h_next ) {
			cvMoments( con0, &mom, 1 );
			d00 = cvGetSpatialMoment( &mom, 0, 0 );
			if ( d00 < 10 )	continue;	// ノイズ（小さい連結成分）の除去
#ifdef	WIN32
			cvDrawContours(smt, con0, cWhite, cWhite, -1, CV_FILLED, 8, cvPoint(0,0) );	// replace CV_FILLED with 1 to see the outlines 
#else
			cvDrawContours( smt, con0, cWhite, cWhite, -1, CV_FILLED, 8 );
#endif
		}
		if ( contours != NULL )	cvClearSeq( contours );
		cvNot( smt, bin );
		//cvNot( bin, inv );

//		// 文字の大きさを調べる
//		cvFindContours( inv, storage, &contours, sizeof(CvContour),
//			CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// 連結成分を抽出する
//	
//		//std::vector<int> area;
//		//CvBox2D  rect;
//		for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
//		{
//			cvMoments(con0, &mom, 1);
//			d00 = cvGetSpatialMoment( &mom, 0, 0 );
//			if ( d00 <= 0.5 ) continue;	// 小さすぎる連結成分は除外
//			//rect = cvMinAreaRect2(con0, NULL);
//			//area.push_back(std::max(rect.size.height, rect.size.width));
//			Add2Hist( &hist, sqrt(d00) );
//		}
//		//std::sort(area.begin(), area.end());
//		//int med = 0;
//		//if ( area.size() % 2 == 0 ) {
//		//	med = area[(area.size()-1)/2];
//		//}
//		//else {
//		//	med = area[(area.size()/2)-1] + area[(area.size()/2)] / 2.0;
//		//}
//		//int odd = med % 2;
//		//med += odd;
//		//med /= 2;
//		//med += med % 2 + 1;
//		//chsize += (chsize + 1) % 2;
//		//chsize = (int)(max_hist * 1.5);	// 12/21変更　高解像度カメラを用いた拡大撮影での結果を精査して
//		//chsize += (chsize + 1) % 2;	// 08/05/30変更　MIRU08再実験のため戻す．精度向上
//		//printf( "med : %d\n", med );
//		chsize += (chsize + 1) % 2;
//		//chsize = (int)(max_hist * 1.5);	// 12/21変更　高解像度カメラを用いた拡大撮影での結果を精査して
//		//chsize += (chsize + 1) % 2;	// 08/05/30変更　MIRU08再実験のため戻す．精度向上

		// 平滑化
		cvSmooth( bin, smt, CV_GAUSSIAN, eMask, 0, 0, 0 );

		// 適応二値化（2回目）
		cvAdaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, eBlockSize, eC );
		cvNot( bin, inv );
		cvClearSeq( contours );
		cvReleaseMemStorage( &storage );
	} else {	// ハッシュ構築モード（電子文書）：閾値固定モード
//		cvThreshold( src, bin, 250, 255, CV_THRESH_BINARY );
//		cvNot( bin, inv );
//
//		//OutPutImage( inv );
//
//		// 輪郭抽出（ノイズ除去のため）
//		cvFindContours( inv, storage, &contours, sizeof(CvContour),
//			CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
//	
//		// 輪郭描画（ノイズ除去のため）
//		int count=0;
//		for ( con0 = contours; con0 != 0; con0 = con0->h_next ) {
//			cvMoments( con0, &mom, 1 );
//			d00 = cvGetSpatialMoment( &mom, 0, 0 );
//			if ( d00 < 10 )	continue;	// ノイズ（小さい連結成分）の除去
//#ifdef	WIN32
//			cvDrawContours(smt, con0, cWhite, cWhite, -1, CV_FILLED, 8, cvPoint(0,0) );	// replace CV_FILLED with 1 to see the outlines 
//#else
//			cvDrawContours( smt, con0, cWhite, cWhite, -1, CV_FILLED, 8 );
//#endif
//			count++;
//		}
//		cout << count << endl;
//		if ( contours != NULL )	cvClearSeq( contours );
//		cvNot( smt, bin );


		cvSmooth( src, smt, CV_GAUSSIAN, 15, 0, 0, 0 );	// OpenCV 1.0
		//OutPutImage( smt );
		//cvThreshold( smt, bin, 250, 255, CV_THRESH_BINARY );
		cvAdaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, eBlockSize, eC );
		//OutPutImage( bin );
		cvNot( bin, inv );
	}
	// あとしまつ
	cvReleaseImage( &src );
	cvReleaseImage( &bin );
	cvReleaseImage( &smt );

	//OutPutImage( inv );
	return inv;
}

IplImage *GetConnectedImageForSmartphone( IplImage *img )
// 結合画像生成
{
//	int i;//, chsize;
	double d00;//, max_hist;
	IplImage *bin, *inv, *smt;
	CvSize img_size;
	CvMemStorage *storage = cvCreateMemStorage(0);
	CvSeq *contours = 0, *con0;
	CvMoments mom;
	strHist hist;
	int i=0, chsize=0;

	// 画像サイズ取得
	img_size = cvGetSize( img );
	// 画像バッファ
	bin = cvCreateImage( img_size, 8, 1 );
	inv = cvCreateImage( img_size, 8, 1 );
	smt = cvCreateImage( img_size, 8, 1 );
	cvZero( bin );
	cvZero( inv );
	cvZero( smt );

	// 適応二値化（1回目）
	cvAdaptiveThreshold( img, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 101, 10 );
	cvNot( bin, inv );

	// 輪郭抽出（ノイズ除去のため）
	cvFindContours( inv, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );

	// 輪郭描画（ノイズ除去のため）
	for ( con0 = contours; con0 != 0; con0 = con0->h_next ) {
		cvMoments( con0, &mom, 1 );
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 < 10 )	continue;	// ノイズ（小さい連結成分）の除去
#ifdef	WIN32
		cvDrawContours(smt, con0, cWhite, cWhite, -1, CV_FILLED, 8, cvPoint(0,0) );	// replace CV_FILLED with 1 to see the outlines 
#else
		cvDrawContours( smt, con0, cWhite, cWhite, -1, CV_FILLED, 8 );
#endif
	}
	if ( contours != NULL )	cvClearSeq( contours );
	cvNot( smt, bin );
	cvNot( bin, inv );

	// 文字の大きさを調べる
	cvFindContours( inv, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// 連結成分を抽出する

	// 連結成分を描画・重心を計算
	InitHist( &hist, 1000, 0, 1000 );
	for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
	{
		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 <= 0.5 ) continue;	// 小さすぎる連結成分は除外
		Add2Hist( &hist, sqrt(d00) );
	}

	chsize = (int)GetMaxBin(&hist);
	//printf( "chsize : %d\n", chsize );
	ReleaseHist( &hist );
	chsize = chsize * 2 + 1;
	//chsize += (chsize + 1) % 2;
	//chsize = (int)(max_hist * 1.5);	// 12/21変更　高解像度カメラを用いた拡大撮影での結果を精査して
	//chsize += (chsize + 1) % 2;	// 08/05/30変更　MIRU08再実験のため戻す．精度向上

	// 平滑化
	cvSmooth( bin, smt, CV_GAUSSIAN, 1, 0, 0, 0 );
	//cvSmooth( bin, smt, CV_GAUSSIAN, chsize, 0, 0, 0 );

	// 適応二値化（2回目）
	cvAdaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 101, 10 );
	cvNot( bin, inv );
	cvClearSeq( contours );
	cvReleaseMemStorage( &storage );

	// あとしまつ
	cvReleaseImage( &img );
	cvReleaseImage( &bin );
	cvReleaseImage( &smt );

	//OutPutImage( inv );
	return inv;
}

int MakeFeaturePoint( IplImage *img, CvPoint **ps0, double **areas0, CvSize *size0 )
	// 特徴点抽出・連結成分の面積計算
{
	int i, num;
	double d00;
	double *areas = NULL;
	CvPoint *ps;
	CvMemStorage *storage = cvCreateMemStorage(0);
	CvSeq *contours = 0, *con0;
	CvMoments mom;
#ifdef DRAW_FP
	IplImage *fp;
#endif

	// 画像サイズの取得
	*size0 = cvGetSize( img );
#ifdef DRAW_FP
	fp = cvCreateImage( *size0, 8, 3 );
#endif

	// 輪郭抽出
	cvFindContours( img, storage, &contours, sizeof(CvContour), 
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0) );

	// 特徴点候補数のカウント
	for ( i = 0, con0 = contours; con0 != 0; con0 = con0->h_next, i++ );
	//printf("%d\n", i);
	num = ( i >= kMaxPointNum ) ? kMaxPointNum - 1 : i;	// ゴミ等で連結成分が多すぎるとき（本来は大きさを調べたりすべき）
	if ( num <= 0 )	return 0;

	// 配列の確保
	ps = (CvPoint *)calloc( num, sizeof(CvPoint) );
	areas = (double *)calloc( num, sizeof(double) );

	// 特徴点抽出・面積計算
	for ( i = 0, con0 = contours; con0 != 0 && i < num ; con0 = con0->h_next ) {
		cvMoments( con0, &mom, 1 );
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 < 5 )	continue;	// 小さい連結成分の除去
		ps[i].x = (int)(cvGetSpatialMoment( &mom, 1, 0 ) / d00 );
		ps[i].y = (int)(cvGetSpatialMoment( &mom, 0, 1 ) / d00 );
		areas[i] = d00;
#ifdef DRAW_FP
		cvCircle( fp, ps[i], 4, cWhite, -1, 1, 0 );
#endif
		i++;
	}

#ifdef DRAW_FP
	cvNot( fp, fp );
	OutPutImage( fp );
	cvReleaseImage( &fp );
#endif
	// 正確な特徴点数に更新
	num = i;

	// あとしまつ
	cvClearSeq( contours );
	cvReleaseMemStorage( &storage );

	// ポインタ渡し
	*ps0 = ps;
	*areas0 = areas;

	return num;
}
