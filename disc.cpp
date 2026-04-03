#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <opencv2/core.hpp>

#ifdef WIN32
#ifdef _DEBUG
#pragma comment( lib, "opencv_core4d.lib" )
#else
#pragma comment( lib, "opencv_core4.lib" )
#endif
#endif

#include "def_general.h"
#include "extern.h"
#include "hist.h"

char Affine2Disc( double cr, strDisc *disc )
// �A���l�̃A�t�B���s�ϗʂ𗣎U�l�ɕϊ�����
{
	if ( cr <= disc->min )		return 0;	// min��菬������΍ŏ��l�ł���0��
	else if ( cr >= disc->max )	return disc->num-1;	// max���傫����΍ő�l�ł���num-1��Ԃ�
	else						return disc->dat[(int)(((cr - disc->min)/(disc->max-disc->min))*(double)disc->res)];	// �����łȂ���ΊY������dat�̒l��Ԃ�
}

char Affine2Disc2( double cr, strDisc *disc )
// �A���l�̃A�t�B���s�ϗʂ𗣎U�l�ɕϊ�����
{
	if ( cr > 1.0 )	{
		cr = 1.0/cr;
		if ( cr <= disc->min )		return 0;	// min��菬������΍ŏ��l�ł���0��
		else if ( cr >= disc->max )	return disc->num-1;	// max���傫����΍ő�l�ł���num-1��Ԃ�
		else						return disc->dat[disc->res - 1 - (int)(((cr - disc->min)/(1.0-disc->min))*(double)disc->res/2.0)];	// �����łȂ���ΊY������dat�̒l��Ԃ�
	} else {
		if ( cr <= disc->min )		return 0;	// min��菬������΍ŏ��l�ł���0��
		else if ( cr >= disc->max )	return disc->num-1;	// max���傫����΍ő�l�ł���num-1��Ԃ�
		else						return disc->dat[(int)(((cr - disc->min)/(1.0-disc->min))*(double)disc->res/2.0)];	// �����łȂ���ΊY������dat�̒l��Ԃ�
	}
}

//char Affine2Disc2( double cr, strDisc *disc )
//// �A���l�̃A�t�B���s�ϗʂ𗣎U�l�ɕϊ�����
//{
//	if ( cr <= disc->min )		return 0;	// min��菬������΍ŏ��l�ł���0��
//	else if ( cr >= disc->max )	return disc->num-1;	// max���傫����΍ő�l�ł���num-1��Ԃ�
//	else if ( cr < 1.0 )		return disc->dat[(int)( (cr - disc->min) / (1.0 - disc->min) * ((double)disc->res / 2.0))];	// �����łȂ���ΊY������dat�̒l��Ԃ�
//	else						return disc->dat[(int)( (cr - 1.0) / (disc->max - 1.0) * ((double)disc->res / 2.0))];	// �����łȂ���ΊY������dat�̒l��Ԃ�
//}

char Affine2DiscForRetrieve( double cr, strDisc *disc, char *vecflag )
// �A���l�̃A�t�B���s�ϗʂ𗣎U�l�ɕϊ� & �����x�N�g�����s�̃t���O����
{
	int idx;
	if ( cr <= disc->min )		return 0;	// min��菬������΍ŏ��l�ł���0��
	else if ( cr >= disc->max )	return disc->num-1;	// max���傫����΍ő�l�ł���num-1��Ԃ�
	else {
		idx = (int)((cr - disc->min)/((disc->max-disc->min)/(double)disc->res));	// �����łȂ���ΊY������dat�̒l��Ԃ�
		if ( idx > eBounds && idx < (disc->res - eBounds) ) {
			if ( disc->dat[idx] > disc->dat[idx-eBounds] ) 		// eBounds��������bin�̗��U�l���قȂ��Ă�����
				*vecflag = -1;
			else if ( disc->dat[idx] < disc->dat[idx+eBounds] ) 	// eBounds�������bin�̗��U�l���قȂ��Ă�����
				*vecflag = 1;
		}
		return disc->dat[idx];
	}
}

void Hist2Disc( strHist *hist, strDisc *disc, int disc_num )
// �q�X�g�O�������痣�U����臒l�����߂�
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
// �q�X�g�O�������痣�U����臒l�����߂�
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
// �q�X�g�O�������J������
{
	free( disc->dat );
}
