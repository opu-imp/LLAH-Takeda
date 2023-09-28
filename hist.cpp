#include <stdio.h>
#include <stdlib.h>

#include "def_general.h"

int InitHist( strHist *hist, int size, double min, double max )
// ヒストグラムを初期化する
{
	if ( size <= 0 || min >= max )	return 0;
	hist->size = size;
	hist->bin = (int *)calloc(size, sizeof(int));
	hist->min = min;
	hist->max = max;

	return 1;
}

void ReleaseHist( strHist *hist )
// ヒストグラムを開放する
{
	free( hist->bin );
}

int Add2Hist( strHist *hist, double cr )
// ヒストグラムにデータを追加する
{
	int idx;

	// オーバーフロー処理
	if (  cr < hist->min || cr >= hist->max )	return 0;

	idx = (int)( (cr - hist->min) / ((hist->max - hist->min) / hist->size) );
	hist->bin[idx]++;

	return 1;
}

int Add2Hist2( strHist *hist, double cr )
// ヒストグラムにデータを追加する
{
	int idx;

	// オーバーフロー処理
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
// ヒストグラムの最大のものをもつbinの右端を返す
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
