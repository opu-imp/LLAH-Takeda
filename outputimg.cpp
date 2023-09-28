#include <stdio.h>
#include "def_general.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "procimg.h"
#include "outputimg.h"


int OutPutImage(IplImage *img)
// 画像をファイルに保存する．ファイル名は自動的に連番になる
{
	static int n = 0;
	char filename[1024];

	sprintf(filename, "output%03d.jpg", n++);

	cvSaveImage( filename, img );
	return 0;
}

int SetPixU( IplImage *img, int x, int y, unsigned char *val )
// imgの(x,y)にvalをセットする(unsigned)
{
	int i, ws, bpp;

	if ( x < 0 || x >= img->width || y < 0 || y >= img->height )	return 0;	// 画像の範囲を超える
	ws = img->widthStep;
	bpp = (int)((img->depth & 0x0ff) / 8) * img->nChannels;	// ピクセルあたりのバイト数
	for ( i = 0; i < bpp; i++ )
		img->imageData[ws*y + x*bpp + i] = val[i];
	return 1;
}

int GetPixU( IplImage *img, int x, int y, unsigned char *val )
// imgの(x,y)をvalにセットする(unsigned)
{
	int i, ws, bpp;

	if ( x < 0 || x >= img->width || y < 0 || y >= img->height )	return 0;	// 画像の範囲を超える
	ws = img->widthStep;
	bpp = (int)((img->depth & 0x0ff) / 8) * img->nChannels;	// ピクセルあたりのバイト数
	for ( i = 0; i < bpp; i++ )
		val[i] = img->imageData[ws*y + x*bpp + i];
	return 1;
}

IplImage *MakeFeaturePointImage( IplImage *orig, IplImage *connected, CvPoint *ps, int num )
// 特徴点画像の作成（入力画像はグレイスケール）
{
	int i, j, p, con_flag = 0;
	IplImage *fp_img, *fp;
	unsigned char val[4];
	cvNot( connected, connected );
	fp_img = cvCreateImage( cvGetSize( orig ), IPL_DEPTH_8U, 3 );
	fp = cvCreateImage( cvGetSize( orig ), IPL_DEPTH_8U, 3 );
	cvZero( fp_img );
	cvNot( fp_img, fp_img );
	if ( fp_img == NULL )	return NULL;
	for ( j = 0; j < orig->height; j++ ) {
		for ( i = 0; i < orig->width; i++ ) {
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
		cvCircle( fp_img, ps[p], 4, cRed, CV_FILLED, CV_AA, 0 );
//		cvCircle( fp_img, ps[p], 4, cWhite, CV_FILLED, CV_AA, 0 );
		//cvCircle( fp, ps[p], 5, cWhite, CV_FILLED, CV_AA, 0 );
	}
	cvNot( fp, fp );
	//OutPutImage(fp);

	return fp_img;
}

#define	kDrawCorHMargin	(100)
#define	kDrawCorVMargin	(100)
#define	kDrawCorSpace	(400)
#define	kDrawCorScale	(0.5)
#define	kDrawCorRectThick	(4)
#define	kDrawCorPtRad	(4)
#define	kDrawCorLineThick	(4)

void DrawCor( CvPoint *ps, int num, CvSize img_size, int res, CvPoint *corps, int cornum, CvSize corsize, int pcor[][2], int pcornum )
{
	int i, width_all, height_all;
	IplImage *img;

	img_size.height = 4368;
	img_size.width = 2912;
	corsize.height = 2200;
	corsize.width = 1700;
	
	width_all = kDrawCorHMargin * 2 + kDrawCorSpace + img_size.width + (int)(corsize.width * kDrawCorScale);
	height_all = kDrawCorVMargin * 2 + std::max( img_size.height, (int)(corsize.height * kDrawCorScale) );
//	printf( "%d,%d\n", width_all, height_all );
	img = cvCreateImage( cvSize( width_all, height_all ), IPL_DEPTH_8U, 3 );
	cvSet( img, cWhite, NULL );	// 白で塗りつぶす

	// 検索質問の枠を描画
	cvRectangle( img, cvPoint( kDrawCorHMargin, kDrawCorVMargin ), \
		cvPoint( kDrawCorHMargin + img_size.width, kDrawCorVMargin + img_size.height ), \
		cBlack, kDrawCorRectThick, CV_AA, 0 );
	
	// 検索質問の特徴点の描画
	for ( i = 0; i < num; i++ ) {
		cvCircle( img, cvPoint( kDrawCorHMargin + ps[i].x, kDrawCorVMargin + ps[i].y ), kDrawCorPtRad, cBlack, -1, CV_AA, 0 );
	}
	// 登録画像の枠を描画
	cvRectangle( img, cvPoint( kDrawCorHMargin + img_size.width + kDrawCorSpace, kDrawCorVMargin), \
		cvPoint( kDrawCorHMargin + img_size.width + kDrawCorSpace + (int)(corsize.width * kDrawCorScale), kDrawCorVMargin + (int)(corsize.height * kDrawCorScale) ), \
		cBlack, kDrawCorRectThick, CV_AA, 0 );
	// 登録画像の特徴点の描画
	for ( i = 0; i < cornum; i++ ) {
		cvCircle( img, cvPoint( kDrawCorHMargin + img_size.width + kDrawCorSpace + (int)(corps[i].x * kDrawCorScale), kDrawCorVMargin + (int)(corps[i].y * kDrawCorScale) ), \
		kDrawCorPtRad, cBlack, -1, CV_AA, 0 );
	}
	// 対応関係の描画
	//for ( i = 0; i < pcornum; i++ ) {
	//	cvLine( img, cvPoint( kDrawCorHMargin + ps[pcor[i][0]].x, kDrawCorVMargin + ps[pcor[i][0]].y ), 
	//	cvPoint( kDrawCorHMargin + img_size.width + kDrawCorSpace + (int)(corps[pcor[i][1]].x * kDrawCorScale), kDrawCorVMargin + (int)(corps[pcor[i][1]].y * kDrawCorScale) ), 
	//	cRed, kDrawCorLineThick, CV_AA, 0 );
	//}
	OutPutImage( img );
	cvReleaseImage( &img );
}
