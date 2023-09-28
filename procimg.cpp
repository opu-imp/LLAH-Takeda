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
// �����摜����
{
	int i, chsize;
	double d00, max_hist;
	IplImage *src, *bin, *inv, *smt;
	CvSize img_size;
	CvMemStorage *storage = cvCreateMemStorage(0);
	CvSeq *contours = 0, *con0;
	CvMoments mom;
	strHist hist;

	// �����摜�ǂݍ���
	if ( (src = cvLoadImage(fname, 0)) == NULL ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		return NULL;
	}
	// �摜�T�C�Y�擾
	img_size = cvGetSize( src );
	// �摜�o�b�t�@
	bin = cvCreateImage( img_size, 8, 1 );
	inv = cvCreateImage( img_size, 8, 1 );
	smt = cvCreateImage( img_size, 8, 1 );
	cvZero( bin );
	cvZero( inv );
	cvZero( smt );

	if ( eEntireMode == RETRIEVE_MODE || eEntireMode == MAKE_QUERY_DAT_MODE ) {
		// �K����l���i1��ځj
		cvAdaptiveThreshold( src, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, eBlockSize, eC );
		cvNot( bin, inv );

		//OutPutImage( inv );

		// �֊s���o�i�m�C�Y�����̂��߁j
		cvFindContours( inv, storage, &contours, sizeof(CvContour),
			CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
	
		// �֊s�`��i�m�C�Y�����̂��߁j
		for ( con0 = contours; con0 != 0; con0 = con0->h_next ) {
			cvMoments( con0, &mom, 1 );
			d00 = cvGetSpatialMoment( &mom, 0, 0 );
			if ( d00 < 10 )	continue;	// �m�C�Y�i�������A�������j�̏���
#ifdef	WIN32
			cvDrawContours(smt, con0, cWhite, cWhite, -1, CV_FILLED, 8, cvPoint(0,0) );	// replace CV_FILLED with 1 to see the outlines 
#else
			cvDrawContours( smt, con0, cWhite, cWhite, -1, CV_FILLED, 8 );
#endif
		}
		if ( contours != NULL )	cvClearSeq( contours );
		cvNot( smt, bin );
		//cvNot( bin, inv );

//		// �����̑傫���𒲂ׂ�
//		cvFindContours( inv, storage, &contours, sizeof(CvContour),
//			CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// �A�������𒊏o����
//	
//		//std::vector<int> area;
//		//CvBox2D  rect;
//		for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
//		{
//			cvMoments(con0, &mom, 1);
//			d00 = cvGetSpatialMoment( &mom, 0, 0 );
//			if ( d00 <= 0.5 ) continue;	// ����������A�������͏��O
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
//		//chsize = (int)(max_hist * 1.5);	// 12/21�ύX�@���𑜓x�J������p�����g��B�e�ł̌��ʂ𐸍�����
//		//chsize += (chsize + 1) % 2;	// 08/05/30�ύX�@MIRU08�Ď����̂��ߖ߂��D���x����
//		//printf( "med : %d\n", med );
//		chsize += (chsize + 1) % 2;
//		//chsize = (int)(max_hist * 1.5);	// 12/21�ύX�@���𑜓x�J������p�����g��B�e�ł̌��ʂ𐸍�����
//		//chsize += (chsize + 1) % 2;	// 08/05/30�ύX�@MIRU08�Ď����̂��ߖ߂��D���x����

		// ������
		cvSmooth( bin, smt, CV_GAUSSIAN, eMask, 0, 0, 0 );

		// �K����l���i2��ځj
		cvAdaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, eBlockSize, eC );
		cvNot( bin, inv );
		cvClearSeq( contours );
		cvReleaseMemStorage( &storage );
	} else {	// �n�b�V���\�z���[�h�i�d�q�����j�F臒l�Œ胂�[�h
//		cvThreshold( src, bin, 250, 255, CV_THRESH_BINARY );
//		cvNot( bin, inv );
//
//		//OutPutImage( inv );
//
//		// �֊s���o�i�m�C�Y�����̂��߁j
//		cvFindContours( inv, storage, &contours, sizeof(CvContour),
//			CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
//	
//		// �֊s�`��i�m�C�Y�����̂��߁j
//		int count=0;
//		for ( con0 = contours; con0 != 0; con0 = con0->h_next ) {
//			cvMoments( con0, &mom, 1 );
//			d00 = cvGetSpatialMoment( &mom, 0, 0 );
//			if ( d00 < 10 )	continue;	// �m�C�Y�i�������A�������j�̏���
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
	// ���Ƃ��܂�
	cvReleaseImage( &src );
	cvReleaseImage( &bin );
	cvReleaseImage( &smt );

	//OutPutImage( inv );
	return inv;
}

IplImage *GetConnectedImageForSmartphone( IplImage *img )
// �����摜����
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

	// �摜�T�C�Y�擾
	img_size = cvGetSize( img );
	// �摜�o�b�t�@
	bin = cvCreateImage( img_size, 8, 1 );
	inv = cvCreateImage( img_size, 8, 1 );
	smt = cvCreateImage( img_size, 8, 1 );
	cvZero( bin );
	cvZero( inv );
	cvZero( smt );

	// �K����l���i1��ځj
	cvAdaptiveThreshold( img, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 101, 10 );
	cvNot( bin, inv );

	// �֊s���o�i�m�C�Y�����̂��߁j
	cvFindContours( inv, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );

	// �֊s�`��i�m�C�Y�����̂��߁j
	for ( con0 = contours; con0 != 0; con0 = con0->h_next ) {
		cvMoments( con0, &mom, 1 );
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 < 10 )	continue;	// �m�C�Y�i�������A�������j�̏���
#ifdef	WIN32
		cvDrawContours(smt, con0, cWhite, cWhite, -1, CV_FILLED, 8, cvPoint(0,0) );	// replace CV_FILLED with 1 to see the outlines 
#else
		cvDrawContours( smt, con0, cWhite, cWhite, -1, CV_FILLED, 8 );
#endif
	}
	if ( contours != NULL )	cvClearSeq( contours );
	cvNot( smt, bin );
	cvNot( bin, inv );

	// �����̑傫���𒲂ׂ�
	cvFindContours( inv, storage, &contours, sizeof(CvContour),
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );	// �A�������𒊏o����

	// �A��������`��E�d�S���v�Z
	InitHist( &hist, 1000, 0, 1000 );
	for( i = 0, con0 = contours; con0 != 0 ; con0 = con0->h_next )
	{
		cvMoments(con0, &mom, 1);
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 <= 0.5 ) continue;	// ����������A�������͏��O
		Add2Hist( &hist, sqrt(d00) );
	}

	chsize = (int)GetMaxBin(&hist);
	//printf( "chsize : %d\n", chsize );
	ReleaseHist( &hist );
	chsize = chsize * 2 + 1;
	//chsize += (chsize + 1) % 2;
	//chsize = (int)(max_hist * 1.5);	// 12/21�ύX�@���𑜓x�J������p�����g��B�e�ł̌��ʂ𐸍�����
	//chsize += (chsize + 1) % 2;	// 08/05/30�ύX�@MIRU08�Ď����̂��ߖ߂��D���x����

	// ������
	cvSmooth( bin, smt, CV_GAUSSIAN, 1, 0, 0, 0 );
	//cvSmooth( bin, smt, CV_GAUSSIAN, chsize, 0, 0, 0 );

	// �K����l���i2��ځj
	cvAdaptiveThreshold( smt, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 101, 10 );
	cvNot( bin, inv );
	cvClearSeq( contours );
	cvReleaseMemStorage( &storage );

	// ���Ƃ��܂�
	cvReleaseImage( &img );
	cvReleaseImage( &bin );
	cvReleaseImage( &smt );

	//OutPutImage( inv );
	return inv;
}

int MakeFeaturePoint( IplImage *img, CvPoint **ps0, double **areas0, CvSize *size0 )
	// �����_���o�E�A�������̖ʐόv�Z
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

	// �摜�T�C�Y�̎擾
	*size0 = cvGetSize( img );
#ifdef DRAW_FP
	fp = cvCreateImage( *size0, 8, 3 );
#endif

	// �֊s���o
	cvFindContours( img, storage, &contours, sizeof(CvContour), 
		CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0) );

	// �����_��␔�̃J�E���g
	for ( i = 0, con0 = contours; con0 != 0; con0 = con0->h_next, i++ );
	//printf("%d\n", i);
	num = ( i >= kMaxPointNum ) ? kMaxPointNum - 1 : i;	// �S�~���ŘA����������������Ƃ��i�{���͑傫���𒲂ׂ��肷�ׂ��j
	if ( num <= 0 )	return 0;

	// �z��̊m��
	ps = (CvPoint *)calloc( num, sizeof(CvPoint) );
	areas = (double *)calloc( num, sizeof(double) );

	// �����_���o�E�ʐόv�Z
	for ( i = 0, con0 = contours; con0 != 0 && i < num ; con0 = con0->h_next ) {
		cvMoments( con0, &mom, 1 );
		d00 = cvGetSpatialMoment( &mom, 0, 0 );
		if ( d00 < 5 )	continue;	// �������A�������̏���
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
	// ���m�ȓ����_���ɍX�V
	num = i;

	// ���Ƃ��܂�
	cvClearSeq( contours );
	cvReleaseMemStorage( &storage );

	// �|�C���^�n��
	*ps0 = ps;
	*areas0 = areas;

	return num;
}
