#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <opencv2/core/core.hpp>

#ifdef WIN32
#ifdef _DEBUG
#pragma comment( lib, "opencv_core231d.lib" )
#else
#pragma comment( lib, "opencv_core231.lib" )
#endif
#endif

#include "def_general.h"
#include "extern.h"
#include "hist.h"

char Affine2Disc( double cr, strDisc *disc )
// 連続値のアフィン不変量を離散値に変換する
{
	if ( cr <= disc->min )		return 0;	// minより小さければ最小値である0を
	else if ( cr >= disc->max )	return disc->num-1;	// maxより大きければ最大値であるnum-1を返す
	else						return disc->dat[(int)(((cr - disc->min)/(disc->max-disc->min))*(double)disc->res)];	// そうでなければ該当するdatの値を返す
}

char Affine2Disc2( double cr, strDisc *disc )
// 連続値のアフィン不変量を離散値に変換する
{
	if ( cr > 1.0 )	{
		cr = 1.0/cr;
		if ( cr <= disc->min )		return 0;	// minより小さければ最小値である0を
		else if ( cr >= disc->max )	return disc->num-1;	// maxより大きければ最大値であるnum-1を返す
		else						return disc->dat[disc->res - 1 - (int)(((cr - disc->min)/(1.0-disc->min))*(double)disc->res/2.0)];	// そうでなければ該当するdatの値を返す
	} else {
		if ( cr <= disc->min )		return 0;	// minより小さければ最小値である0を
		else if ( cr >= disc->max )	return disc->num-1;	// maxより大きければ最大値であるnum-1を返す
		else						return disc->dat[(int)(((cr - disc->min)/(1.0-disc->min))*(double)disc->res/2.0)];	// そうでなければ該当するdatの値を返す
	}
}

//char Affine2Disc2( double cr, strDisc *disc )
//// 連続値のアフィン不変量を離散値に変換する
//{
//	if ( cr <= disc->min )		return 0;	// minより小さければ最小値である0を
//	else if ( cr >= disc->max )	return disc->num-1;	// maxより大きければ最大値であるnum-1を返す
//	else if ( cr < 1.0 )		return disc->dat[(int)( (cr - disc->min) / (1.0 - disc->min) * ((double)disc->res / 2.0))];	// そうでなければ該当するdatの値を返す
//	else						return disc->dat[(int)( (cr - 1.0) / (disc->max - 1.0) * ((double)disc->res / 2.0))];	// そうでなければ該当するdatの値を返す
//}

char Affine2DiscForRetrieve( double cr, strDisc *disc, char *vecflag )
// 連続値のアフィン不変量を離散値に変換 & 複数ベクトル発行のフラグ立て
{
	int idx;
	if ( cr <= disc->min )		return 0;	// minより小さければ最小値である0を
	else if ( cr >= disc->max )	return disc->num-1;	// maxより大きければ最大値であるnum-1を返す
	else {
		idx = (int)((cr - disc->min)/((disc->max-disc->min)/(double)disc->res));	// そうでなければ該当するdatの値を返す
		if ( idx > eBounds && idx < (disc->res - eBounds) ) {
			if ( disc->dat[idx] > disc->dat[idx-eBounds] ) 		// eBoundsだけ下のbinの離散値が異なっていたら
				*vecflag = -1;
			else if ( disc->dat[idx] < disc->dat[idx+eBounds] ) 	// eBoundsだけ上のbinの離散値が異なっていたら
				*vecflag = 1;
		}
		return disc->dat[idx];
	}
}

void Hist2Disc( strHist *hist, strDisc *disc, int disc_num )
// ヒストグラムから離散化の閾値を求める
{
	int i, j;
	unsigned int total, sum;
	
	disc->min = hist->min;
	disc->max = hist->max;
	disc->num = disc_num;
	disc->res = hist->size;
	disc->dat = (int *)calloc(disc->res, sizeof(int));
	for ( i = 0, total = 0; i < hist->size; i++ ) {
		total += (unsigned int)hist->bin[i];
	}
	for ( i = 0, j = 0, sum = 0; i < hist->size; i++ ) {
		disc->dat[i] = j;
		sum += (unsigned int)hist->bin[i];
		if ( (sum) > (unsigned int)((double)total/ ((double)disc_num / (double)(j+1))) )	j++;
	}
}

void Hist2Disc2( strHist *hist, strDisc *disc, int disc_num )
// ヒストグラムから離散化の閾値を求める
{
	int i, j;
	unsigned int total=0, sum=0;
	
	disc->min = hist->min;
	disc->max = hist->max;
	disc->num = disc_num;
	disc->res = hist->size;
	disc->dat = (int *)calloc(disc->res, sizeof(int));

	for ( i = 0; i < hist->size; i++ )	total += (unsigned int)hist->bin[i];
	double border_unit = (double)total/(double)disc_num;

	for ( i = 0, j = 0; i < hist->size; i++ ) {
		disc->dat[i] = j;
		sum += (unsigned int)hist->bin[i];
		if ( sum > (unsigned int)( border_unit*(double)(j+1)) )	j++;
	}
}

void ReleaseDisc( strDisc *disc )
// ヒストグラムを開放する
{
	free( disc->dat );
}
