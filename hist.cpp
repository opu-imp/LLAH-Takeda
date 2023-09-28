#include <stdio.h>
#include <stdlib.h>

#include "def_general.h"

int InitHist( strHist *hist, int size, double min, double max )
// �q�X�g�O����������������
{
	if ( size <= 0 || min >= max )	return 0;
	hist->size = size;
	hist->bin = (int *)calloc(size, sizeof(int));
	hist->min = min;
	hist->max = max;

	return 1;
}

void ReleaseHist( strHist *hist )
// �q�X�g�O�������J������
{
	free( hist->bin );
}

int Add2Hist( strHist *hist, double cr )
// �q�X�g�O�����Ƀf�[�^��ǉ�����
{
	int idx;

	// �I�[�o�[�t���[����
	if (  cr < hist->min || cr >= hist->max )	return 0;

	idx = (int)( (cr - hist->min) / ((hist->max - hist->min) / hist->size) );
	hist->bin[idx]++;

	return 1;
}

int Add2Hist2( strHist *hist, double cr )
// �q�X�g�O�����Ƀf�[�^��ǉ�����
{
	int idx;

	// �I�[�o�[�t���[����
	if (  cr < hist->min || cr >= hist->max )	return 0;

	if ( cr < 1.0 ) {
		idx = (int)( (cr - hist->min) / (1.0 - hist->min) * (hist->size/2.0) );
		hist->bin[idx]++;
	} else {
		cr = 1.0 / cr;
		idx = (int)( (cr - hist->min) / (1.0 - hist->min) * (hist->size/2.0) );
		hist->bin[hist->size - 1 - idx]++;
	}

	return 1;
}

double GetMaxBin( strHist *hist )
// �q�X�g�O�����̍ő�̂��̂�����bin�̉E�[��Ԃ�
{
	int i, max = 0, max_bin = 0;
	
	for ( i = 0; i < hist->size; i++ ) {
		if ( hist->bin[i] > max ) {
			max = hist->bin[i];
			max_bin = i;
		}
	}
	return (((hist->max - hist->min) / (double)hist->size) * (double)(max_bin+1));
}
