#include "def_general.h"

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui_c.h>

#ifdef WIN32
#ifdef _DEBUG
#pragma comment( lib, "opencv_core231d.lib" )
#else
#pragma comment( lib, "opencv_core231.lib" )
#endif
#endif

#include "extern.h"
#include "hist.h"
#include "disc.h"
#include "hash.h"
#include "outputimg.h"

extern int **nears;

long double GetPointsAngle(CvPoint p1, CvPoint p2)
// p1から見たp2の角度を計算する（約-3.14から約3.14）
{
	return atan2((double)(p2.y-p1.y), (double)(p2.x-p1.x));
}

void CalcOrderCWN( int pt, CvPoint *ps, int idx[], int num )
// 時計回りに近傍点を配列に入れる（ｎ点バージョン）
{
	int i, j, tmp_i;
	double *angs, tmp_a;

	angs = (double *)calloc( num, sizeof(double) );
	for ( i = 0; i < num; i++ ) {
		angs[i] = GetPointsAngle(ps[pt], ps[nears[pt][i]]);
		if ( angs[i] < 0.0 )	angs[i] += PI * 2.0;
		idx[i] = nears[pt][i];
	}
	for ( i = 1; i <= num-2; i++ ) {
		for ( j = num-1; j > i; j-- ) {
			if ( angs[j] < angs[i] ) {
				tmp_a = angs[i];
				angs[i] = angs[j];
				angs[j] = tmp_a;
				tmp_i = idx[i];
				idx[i] = idx[j];
				idx[j] = tmp_i;
			}
		}
	}
	free(angs);
}

int FindStartPoint( char *inv_array, int st_num )
// 開始点を見つける
{
	int i, j, min;

	for ( i = 1, min = 0; i < st_num; i++ ) {
		// まずinv_array[min]とinv_array[i]を比較し，
		// 次にinv_array[min+1]とinv_array[i+1]というように順に比較して小さいほうをminとする．
		// 折り返しのためeGroup2Numでmodを取る
		for ( j = 0; j < st_num; j++ ) {
			if ( inv_array[(min+j)%st_num] < inv_array[(i+j)%st_num] ) {
				break;
			} else if ( inv_array[(min+j)%st_num] > inv_array[(i+j)%st_num] ) {
				min = i;
				break;
			}
		}
	}
	return min;
}

double CalcArea( CvPoint p1, CvPoint p2, CvPoint p3 )
// 指定された3点で囲まれた三角形の面積を計算する
{
	return fabs((double)((p1.x-p3.x)*(p2.y-p3.y)-(p1.y-p3.y)*(p2.x-p3.x))) / (double)2.0L;
}

double CalcAffineInv( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4 )
// 平面上の4点でアフィン変換の不変量を計算する
{
	double s;
	if ( (s = CalcArea(p1, p2, p3)) < 0.0001 )	s = 0.0001;
	return CalcArea(p1, p3, p4) / s ;
}

void CalcHindexAreaRatio( int *idxcom1, double *areas, char *hindex_area )
// 面積からインデックスを計算
// 面積比の順位バージョン
// 
{
	int i, j;
	int *num_array, tmp_num;
	double *ratio_array, tmp_area;

	num_array = (int *)calloc( eGroup2Num, sizeof(int) );
	ratio_array = (double *)calloc( eGroup2Num, sizeof(double) );

	for ( i = 0; i < eGroup2Num; i++ ) {
		num_array[i] = i;
		if ( i != eGroup2Num-1 ){
			ratio_array[i] = areas[idxcom1[i]]/areas[idxcom1[i+1]];
		}
	}

	ratio_array[eGroup2Num-1] = areas[idxcom1[eGroup2Num-1]] / areas[idxcom1[0]];

	for ( i = 0; i < eGroup2Num-1; i++ ) {
		for ( j = i+1; j < eGroup2Num; j++ ) {
			if ( ratio_array[i] < ratio_array[j] ) {
				tmp_num = num_array[i];
				num_array[i] = num_array[j];
				num_array[j] = tmp_num;
				tmp_area = ratio_array[i];
				ratio_array[i] = ratio_array[j];
				ratio_array[j] = tmp_area;
			}
		}
	}
	for ( i = 0; i < eGroup2Num; i++ ){
		hindex_area[i] = (char)num_array[i];
	}
	free( num_array );
	free( ratio_array );
}


void CalcAffineAndAddHist( CvPoint *ps, int num, strHist *hist )
// アフィン不変量を計算してヒストグラムに入れる
/****************************************************************/
/*	複数特徴ベクトルを計算するのは，特徴点の隠れに対応するため	*/
/*  中心点は隠れないので，中心点を含まない組合せは必要ない？	*/
/****************************************************************/
{
	int i, j, k, l, st, tmp, vec_num, st_num;
	int *idx, *idxcom1, *idxcom2;
	double cr;

	// 配列の確保
	idx		= (int *)calloc( eGroup1Num, sizeof(int) );
	idxcom1	= (int *)calloc( eGroup2Num, sizeof(int) );
	idxcom2	= (int *)calloc( eGroup3Num, sizeof(int) );

	vec_num = eNumCom1-1;
	st_num = eGroup2Num-1;

	for ( i = 0; i < num; i++ ) {	// 全ての特徴点について
		// 近傍点を時計回りに整列
		CalcOrderCWN( i, ps, idx, eGroup1Num );
		for ( j = 0; j < vec_num; j++ ) {	// nCm-1
			for ( k = 0; k < eGroup2Num; k++ )	idxcom1[k] = idx[com1[j][k]];	// m点の抽出
			for ( st = 0; st < st_num; st++ ) {	// 開始点
				for ( k = 0; k < eNumCom2; k++ ) {	// mC4
					for ( l = 0; l < eGroup3Num; l++ )	idxcom2[l] = idxcom1[com2[k][l]];
					cr = CalcAffineInv( ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]], ps[idxcom2[3]]);
					// ヒストグラムに追加
					if (eTrueHist)	Add2Hist2( hist, cr );
					else			Add2Hist( hist, cr );
				}

				// 回転
				tmp = idxcom1[1];
				for ( k = 1; k < st_num; k++ )	idxcom1[k] = idxcom1[k+1];
				idxcom1[st_num] = tmp;
			}
		}
	}

	// 解放
	free( idx );
	free( idxcom1 );
	free( idxcom2 );
}

void CalcAffineRotateOnceAndAddHash( CvPoint *ps, double *areas, int n, int num, strDisc *disc, strHash **hash, int *pids, int *collision )
// アフィン不変量計算
// 衝突回数カウント
{
	char *hindex = NULL, *hindex_area = NULL, *inv_array = NULL;
	int i, j, k, l, st, vec_num, st_num;
	int *idx = NULL, *idxcom1 = NULL, *idxcom2 = NULL, *idxcom1bak = NULL;
	unsigned long long int index;
	unsigned char quotient;
	double cr;
	
	vec_num = eNumCom1-1;
	st_num = eGroup2Num-1;

	// 配列の確保
	idx = ( int * )calloc( eGroup1Num, sizeof(int) );
	idxcom1 = ( int * )calloc( eGroup2Num, sizeof(int) );
	idxcom1bak = ( int * )calloc( st_num, sizeof(int) );
	idxcom2 = ( int * )calloc( eGroup3Num, sizeof(int) );
	hindex = ( char * )calloc( eNumCom2, sizeof(char) );
	hindex_area = ( char * )calloc( eGroup2Num, sizeof(char) );
	inv_array = ( char * )calloc( st_num, sizeof(char) );

	//FILE *fp;
	//if ( (fp = fopen( "affine.txt", "a" )) == NULL ) {
	//	fprintf( stderr, "error : %s cannot be opened\n", "affine.txt" );
	//}

	for ( i = 0; i < num; i++ ) {	// for all points
		// 近傍点を時計回りに整列
		CalcOrderCWN( pids[i], ps, idx, eGroup1Num );
		for ( j = 0; j < vec_num; j++ )	{	// nCm
		//for ( j = 0; j < 2; j++ )	{	// nCm
			for ( k = 0; k < eGroup2Num; k++ )	idxcom1[k] = idx[com1[j][k]];	// m点の抽出

			/************** 開始点の探索 **************************************************/
			// 不変量の計算
			for ( st = 0; st < st_num; st++ ) {
				cr = CalcAffineInv( ps[idxcom1[0]], ps[idxcom1[(st%st_num)+1]], ps[idxcom1[((st+1)%st_num)+1]], ps[idxcom1[((st+2)%st_num)+1]]);
				//fprintf( fp, "%lf\n", cr );	// 元画像ファイル名の出力
				if ( eTrueHist )	inv_array[st] = Affine2Disc2( cr, disc );
				else				inv_array[st] = Affine2Disc( cr, disc );
			}
			
			// 不変量から開始点を見つける
			st = FindStartPoint( inv_array, st_num );
			// stを開始点として回転させる
			for ( k = 0; k < st_num; k++ )	idxcom1bak[k] = idxcom1[k+1];
			for ( k = 0; k < st_num; k++ )	idxcom1[k+1] = idxcom1bak[(k+st)%st_num];
			/************** 探索の終了 ****************************************************/

			// アフィン不変量を離散化
			for ( k = 0; k < eNumCom2; k++ ) {
				for ( l = 0; l < eGroup3Num; l++ )	idxcom2[l] = idxcom1[com2[k][l]];
				cr = CalcAffineInv(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]], ps[idxcom2[3]]);
				if (eTrueHist)	hindex[k] = Affine2Disc2( cr, disc );
				else			hindex[k] = Affine2Disc( cr, disc );
				//printf("%d ", hindex[k]);
			}
			//puts("");
			// ハッシュ表のインデックスを計算
			if ( eUseArea ) {	// 面積特徴量追加
				CalcHindexAreaRatio( idxcom1, areas, hindex_area );
				index = HashFuncArea( hindex, hindex_area, disc->num, &quotient );
			}
			else {
				index = HashFunc( hindex, disc->num, &quotient );
			}
			if ( eCompressHash )	AddHashCompress( index, n, i, quotient, hash );
			else 					AddHash( index, n, i, quotient, hash );
		}
	}


	//fclose(fp);

	free( hindex );
	free( hindex_area );
	free( idx );
	free( idxcom1 );
	free( idxcom2 );
	free( idxcom1bak );
	free( inv_array );
}

void AdditionalAffineToHash( CvPoint *ps, double *areas, int n, int num, strDisc *disc, strHash **hash, int *pids, int *collision )
// アフィン不変量計算
// 衝突回数カウント
{
	char *hindex = NULL, *hindex_area = NULL, *inv_array = NULL;
	int i, j, k, l, st, vec_num, st_num;
	int *idx = NULL, *idxcom1 = NULL, *idxcom2 = NULL, *idxcom1bak = NULL;
	unsigned long long int index;
	unsigned char quotient;
	double cr;
	
	vec_num = eNumCom1-1;
	st_num = eGroup2Num-1;

	// 配列の確保
	idx = ( int * )calloc( eGroup1Num, sizeof(int) );
	idxcom1 = ( int * )calloc( eGroup2Num, sizeof(int) );
	idxcom1bak = ( int * )calloc( st_num, sizeof(int) );
	idxcom2 = ( int * )calloc( eGroup3Num, sizeof(int) );
	hindex = ( char * )calloc( eNumCom2, sizeof(char) );
	hindex_area = ( char * )calloc( eGroup2Num, sizeof(char) );
	inv_array = ( char * )calloc( st_num, sizeof(char) );

	//FILE *fp;
	//if ( (fp = fopen( "affine.txt", "a" )) == NULL ) {
	//	fprintf( stderr, "error : %s cannot be opened\n", "affine.txt" );
	//}

	for ( i = 0; i < num; i++ ) {	// for all points
		// 近傍点を時計回りに整列
		CalcOrderCWN( pids[i], ps, idx, eGroup1Num );
		//for ( j = 0; j < vec_num; j++ )	{	// nCm
		for ( j = 0; j < 2; j++ )	{	// nCm
			for ( k = 0; k < eGroup2Num; k++ )	idxcom1[k] = idx[com1[j][k]];	// m点の抽出

			/************** 開始点の探索 **************************************************/
			// 不変量の計算
			for ( st = 0; st < st_num; st++ ) {
				cr = CalcAffineInv( ps[idxcom1[0]], ps[idxcom1[(st%st_num)+1]], ps[idxcom1[((st+1)%st_num)+1]], ps[idxcom1[((st+2)%st_num)+1]]);
				//fprintf( fp, "%lf\n", cr );	// 元画像ファイル名の出力
				if ( eTrueHist )	inv_array[st] = Affine2Disc2( cr, disc );
				else				inv_array[st] = Affine2Disc( cr, disc );
			}
			
			// 不変量から開始点を見つける
			st = FindStartPoint( inv_array, st_num );
			// stを開始点として回転させる
			for ( k = 0; k < st_num; k++ )	idxcom1bak[k] = idxcom1[k+1];
			for ( k = 0; k < st_num; k++ )	idxcom1[k+1] = idxcom1bak[(k+st)%st_num];
			/************** 探索の終了 ****************************************************/

			// アフィン不変量を離散化
			for ( k = 0; k < eNumCom2; k++ ) {
				for ( l = 0; l < eGroup3Num; l++ )	idxcom2[l] = idxcom1[com2[k][l]];
				cr = CalcAffineInv(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]], ps[idxcom2[3]]);
				if (eTrueHist)	hindex[k] = Affine2Disc2( cr, disc );
				else			hindex[k] = Affine2Disc( cr, disc );
				//printf("%d ", hindex[k]);
			}
			//puts("");
			// ハッシュ表のインデックスを計算
			if ( eUseArea ) {	// 面積特徴量追加
				CalcHindexAreaRatio( idxcom1, areas, hindex_area );
				index = HashFuncArea( hindex, hindex_area, disc->num, &quotient );
			}
			else {
				index = HashFunc( hindex, disc->num, &quotient );
			}
			if ( eCompressHash )	AddHashCompress( index, n, i, quotient, hash );
			else 					AddHash( index, n, i, quotient, hash );
		}
	}

	//fclose(fp);

	free( hindex );
	free( hindex_area );
	free( idx );
	free( idxcom1 );
	free( idxcom2 );
	free( idxcom1bak );
	free( inv_array );
}

void CalcAffineRotateOnce( CvPoint *ps, double *areas, int n, int num, strDisc *disc, strHash **hash, double ***features, char ***area_features )
// アフィン不変量計算
// 衝突回数カウント
{
	char *hindex_area = NULL, *inv_array = NULL;
	int i, j, k, l, st, vec_num, st_num;
	int *idx = NULL, *idxcom1 = NULL, *idxcom2 = NULL, *idxcom1bak = NULL;
	double cr;
	
	vec_num = eNumCom1-1;
	st_num = eGroup2Num-1;

	// 配列の確保
	idx = ( int * )calloc( eGroup1Num, sizeof(int) );
	idxcom1 = ( int * )calloc( eGroup2Num, sizeof(int) );
	idxcom1bak = ( int * )calloc( st_num, sizeof(int) );
	idxcom2 = ( int * )calloc( eGroup3Num, sizeof(int) );
	hindex_area = ( char * )calloc( eGroup2Num, sizeof(char) );
	inv_array = ( char * )calloc( st_num, sizeof(char) );

	for ( i = 0; i < num; i++ ) {	// for all points
		// 近傍点を時計回りに整列
		CalcOrderCWN( i, ps, idx, eGroup1Num );
		for ( j = 0; j < vec_num; j++ )	{	// nCm
			for ( k = 0; k < eGroup2Num; k++ )	idxcom1[k] = idx[com1[j][k]];	// m点の抽出

			/************** 開始点の探索 **************************************************/
			// 不変量の計算
			for ( st = 0; st < st_num; st++ ) {
				cr = CalcAffineInv( ps[idxcom1[0]], ps[idxcom1[(st%st_num)+1]], ps[idxcom1[((st+1)%st_num)+1]], ps[idxcom1[((st+2)%st_num)+1]]);
				if ( eTrueHist )	inv_array[st] = Affine2Disc2( cr, disc );
				else				inv_array[st] = Affine2Disc( cr, disc );
			}
			
			// 不変量から開始点を見つける
			st = FindStartPoint( inv_array, st_num );
			// stを開始点として回転させる
			for ( k = 0; k < st_num; k++ )	idxcom1bak[k] = idxcom1[k+1];
			for ( k = 0; k < st_num; k++ )	idxcom1[k+1] = idxcom1bak[(k+st)%st_num];
			/************** 探索の終了 ****************************************************/

			// アフィン不変量
			for ( k = 0; k < eNumCom2; k++ ) {
				for ( l = 0; l < eGroup3Num; l++ )	idxcom2[l] = idxcom1[com2[k][l]];
				features[i][j][k] = CalcAffineInv(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]], ps[idxcom2[3]]);
				//if ( features[i][j][k] > 10.0 )	features[i][j][k] = 10.0;
			}
			// 面積特徴量計算
			if ( eUseArea ) {	// 面積特徴量追加
				CalcHindexAreaRatio( idxcom1, areas, hindex_area );
				memcpy(area_features[i][j], hindex_area, eGroup2Num);
			}
		}
	}

	free( hindex_area );
	free( idx );
	free( idxcom1 );
	free( idxcom2 );
	free( idxcom1bak );
	free( inv_array );
}


void CalcAffineRotateOnce2( CvPoint *ps, double *areas, int n, int num, strDisc *disc, strHash **hash, double ***features, char ***area_features, int *pids )
// アフィン不変量計算
// 衝突回数カウント
{
	char *hindex_area = NULL, *inv_array = NULL;
	int i, j, k, l, st, vec_num, st_num;
	int *idx = NULL, *idxcom1 = NULL, *idxcom2 = NULL, *idxcom1bak = NULL;
	double cr;
	
	vec_num = eNumCom1-1;
	st_num = eGroup2Num-1;

	// 配列の確保
	idx = ( int * )calloc( eGroup1Num, sizeof(int) );
	idxcom1 = ( int * )calloc( eGroup2Num, sizeof(int) );
	idxcom1bak = ( int * )calloc( st_num, sizeof(int) );
	idxcom2 = ( int * )calloc( eGroup3Num, sizeof(int) );
	hindex_area = ( char * )calloc( eGroup2Num, sizeof(char) );
	inv_array = ( char * )calloc( st_num, sizeof(char) );

	for ( i = 0; i < num; i++ ) {	// for all points
		// 近傍点を時計回りに整列
		CalcOrderCWN( pids[i], ps, idx, eGroup1Num );
		for ( j = 0; j < vec_num; j++ )	{	// nCm
			for ( k = 0; k < eGroup2Num; k++ )	idxcom1[k] = idx[com1[j][k]];	// m点の抽出

			/************** 開始点の探索 **************************************************/
			// 不変量の計算
			for ( st = 0; st < st_num; st++ ) {
				cr = CalcAffineInv( ps[idxcom1[0]], ps[idxcom1[(st%st_num)+1]], ps[idxcom1[((st+1)%st_num)+1]], ps[idxcom1[((st+2)%st_num)+1]]);
				if ( eTrueHist )	inv_array[st] = Affine2Disc2( cr, disc );
				else				inv_array[st] = Affine2Disc( cr, disc );
			}
			
			// 不変量から開始点を見つける
			st = FindStartPoint( inv_array, st_num );
			// stを開始点として回転させる
			for ( k = 0; k < st_num; k++ )	idxcom1bak[k] = idxcom1[k+1];
			for ( k = 0; k < st_num; k++ )	idxcom1[k+1] = idxcom1bak[(k+st)%st_num];
			/************** 探索の終了 ****************************************************/

			// アフィン不変量
			for ( k = 0; k < eNumCom2; k++ ) {
				for ( l = 0; l < eGroup3Num; l++ )	idxcom2[l] = idxcom1[com2[k][l]];
				features[i][j][k] = CalcAffineInv(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]], ps[idxcom2[3]]);
				//if ( features[i][j][k] > 10.0 )	features[i][j][k] = 10.0;
			}
			// 面積特徴量計算
			if ( eUseArea ) {	// 面積特徴量追加
				CalcHindexAreaRatio( idxcom1, areas, hindex_area );
				memcpy(area_features[i][j], hindex_area, eGroup2Num);
			}
		}
	}

	free( hindex_area );
	free( idx );
	free( idxcom1 );
	free( idxcom2 );
	free( idxcom1bak );
	free( inv_array );
}

double CalcFeatureScore(double feature, double *border, strDisc *disc)
{
	double x=0.0;
	if ( feature < border[0] ) {					// 離散化領域が左端の場合
		if ( feature < border[0]/2.0 )	x = 0.0;
		else										x = (feature / border[0] * 2 - 1.0) * 3.0;
	}
	else if ( feature > border[disc->num-2] ) {		// 離散化領域が右端の場合
		if ( feature > (disc->max + border[disc->num-2])/2.0 )	x = 0.0;
		else	x = ((feature - border[disc->num-2]) / (disc->max-border[disc->num-2]) * 2 - 1.0)*3.0;
	} else {
		for ( int l = 0; l < disc->num-1; l++ ) {
			if ( feature >= border[l] && feature < border[l+1] ) {
				x = ((feature - border[l]) / (border[l+1]-border[l]) * 2 - 1.0)*3.0;
				break;
			}
		}
	}
	double score = exp(-x*x/2.0);

	return score;
}

void RankFeatureAndAddHash(double ***features, char ***area_features, int doc_id, int p_num, strDisc *disc, double *areas, strHash **hash, int tx, int ty, int ***div_ps, int **div_pnum, CvPoint *ps)
{
	unsigned long long int index;
	unsigned char quotient;
	int feature_num = eFeatureNum;
	//IplImage *img;
	//img = cvLoadImage("/home/takeda/code/llahdoc/0606/test/cvpr01p1010.jpg", 1);
	//img = cvCreateImage( cvSize(1700,2200), IPL_DEPTH_8U, 3 );
	//cvSet( img, cWhite, NULL );	// 白で塗りつぶす

	int *bindex = (int *)calloc(disc->num-1, sizeof(int));					// 離散化テーブルの閾値ID
	double *border = (double *)calloc(disc->num-1, sizeof(double));			// 閾値IDにおける不変量値

	// 離散化テーブルの閾値インデックスの取得
	for ( int i = 0, j = 0; i < disc->res; i++ ) {
		if( disc->dat[i] != j ) {
			bindex[j] = i;
			j++;
		}
	}
	// 閾値インデックスのアフィン不変量を算出
	for ( int i = 0; i < disc->num-1; i++ ) {
		border[i] = bindex[i] * (disc->max-disc->min)/disc->res + disc->min;
	}

	// 特徴量の各次元の得点配列
	double ***f_score = (double ***)calloc(p_num, sizeof(double **));
	for ( int i = 0; i < p_num; i++ ) {
		f_score[i] = (double **)calloc(eNumCom1-1, sizeof(double *));
		for ( int j = 0; j < eNumCom1-1; j++ ) {
			f_score[i][j] = (double *)calloc(eNumCom2, sizeof(double));
		}
	}

	// 特徴量の削除フラグ
	char **f_flag = (char **)calloc(p_num, sizeof(char *));
	for ( int i = 0; i < p_num; i++ ) {
		f_flag[i] = (char *)calloc(eNumCom1-1, sizeof(char));
	}
	int f_num=p_num*(eNumCom1-1);
	// 特徴量の得点計算
	for ( int i = 0; i < p_num; i++ ) {				// 各特徴点
		for ( int j = 0; j < eNumCom1-1; j++ ) {	// 特徴点ごとの特徴量
			for ( int k = 0; k < eNumCom2; k++ ) {	// 特徴量毎の各次元
				f_score[i][j][k] = CalcFeatureScore( features[i][j][k], border, disc );
				if ( features[i][j][k] > 100.0 || features[i][j][k] < 0.001 ) {
					f_flag[i][j] = 1;
					f_num--;
					break;
				}
			}
		}
	}
	//int flag = 0;
	//int size = eNumCom2*sizeof(double);
	//for ( int i = 0; i < p_num-1; i++ ) {				// 各特徴点
	//	for ( int j = 0; j < eNumCom1-2; j++ ) {	// 特徴点ごとの特徴量
	//		flag = 0;
	//		for ( int k = i+1; k < p_num; k++ ) {
	//			for ( int l = j+1; l < eNumCom1-1; l++ ) {
	//				if ( !memcmp(features[i][j], features[k][l], size) ) {
	//					for ( int m = 0; m < eNumCom2; m++ ) printf("%9.6lf ", features[i][j][m]);
	//					puts("");
	//					for ( int m = 0; m < eNumCom2; m++ ) printf("%9.6lf ", features[k][l][m]);
	//					puts("");
	//					//printf("match\n");
	//					f_flag[i][j] = 1;
	//					flag = 1;
	//					f_num--;
	//					break;
	//				}
	//			}
	//			if ( flag == 1 )	break;
	//		}
	//	}
	//}

	int f_num_bak = f_num;
	double threshold = 0.01;
	double rate = 0.01;
	while ( 1 ) {
		f_num = f_num_bak;
		for ( int i = 0; i < p_num; i++ ) {				// 各特徴点
			for ( int j = 0; j < eNumCom1-1; j++ ) {	// 特徴点ごとの特徴量
				if ( f_flag[i][j] == 0 ) {
					for ( int k = 0; k < eNumCom2; k++ ) {	// 特徴量毎の各次元
						if ( f_score[i][j][k] < threshold ) {
							f_num--;
							break;
						}
					}
				}
			}
		}
		//printf("t: %10.7lf, rate: %lf, num: %d\n", threshold, rate, f_num);
		if ( f_num == eFeatureNum )	break;
		if ( f_num < eFeatureNum ) {
			if ( rate < 0.000001 )	break;
			else {
				threshold -= rate;
				rate *= 0.1;
			}
		}
		threshold += rate;
	}

	for ( int i = 0; i < p_num; i++ ) {				// 各特徴点
		for ( int j = 0; j < eNumCom1-1; j++ ) {	// 特徴点ごとの特徴量
			if ( f_flag[i][j] == 0 ) {
				for ( int k = 0; k < eNumCom2; k++ ) {	// 特徴量毎の各次元
					if ( f_score[i][j][k] < threshold ) {
						f_flag[i][j] = 1;
						break;
					}
				}
			}
		}
	}

	char *hindex = ( char * )calloc( eNumCom2, sizeof(char) );
	for ( int i = 0; i < p_num; i++ ) {				// 各特徴点
		for ( int j = 0; j < eNumCom1-1; j++ ) {	// 特徴点ごとの特徴量
			if ( f_flag[i][j] == 0 ) {
				//cvCircle( img, ps[i], 12, cRed, CV_FILLED, CV_AA, 0 );
				for ( int k = 0; k < eNumCom2; k++ ) {	// 特徴量毎の各次元
					if (eTrueHist)	hindex[k] = Affine2Disc2( features[i][j][k], disc );
					else			hindex[k] = Affine2Disc( features[i][j][k], disc );
				}
				// ハッシュ表のインデックスを計算
				if ( eUseArea )	index = HashFuncArea( hindex, area_features[i][j], disc->num, &quotient );
				else			index = HashFunc( hindex, disc->num, &quotient );
			//printf("%llu, %u\n", index, quotient);

				if ( eCompressHash )	AddHashCompress( index, doc_id, i, quotient, hash );
				else 					AddHash( index, doc_id, i, quotient, hash );
			}
		}
	}

	//OutPutImage(img);

	//cvReleaseImage( &img );
	// メモリ解放
	for ( int i = 0; i < p_num; i++ ) {
		for ( int j = 0; j < eNumCom1-1; j++ ) {
			free(f_score[i][j]);
		}
		free(f_score[i]);
		free(f_flag[i]);
	}
	free(f_score);
	free(f_flag);
	free( bindex );	// 特徴量の得点
	free( border );	// 特徴量の得点
	free( hindex );
}

//void RankFeatureAndAddHash(double ***features, char ***area_features, int doc_id, int p_num, strDisc *disc, double *areas, strHash **hash, int tx, int ty, int ***div_ps, int **div_pnum)
//{
//	double x;
//	int n=0;
//	unsigned int index;
//	unsigned char quotient;
//
//	double *fx	= (double *)calloc(p_num*(eNumCom1-1), sizeof(double));		// 特徴量の得点
//	int *p_id		= (int *)	calloc(p_num*(eNumCom1-1), sizeof(int));	// 特徴量の特徴点ID
//	int *f_id		= (int *)	calloc(p_num*(eNumCom1-1), sizeof(int));	// 特徴量の特徴点ID
//	int *bindex = (int *)calloc(disc->num-1, sizeof(int));					// 離散化テーブルの閾値ID
//	double *border = (double *)calloc(disc->num-1, sizeof(double));			// 閾値IDにおける不変量値
//	char *hindex = ( char * )calloc( eNumCom2, sizeof(char) );
//
//	//for ( int i = 0; i < p_num; i++ ) {
//	//	for ( int j = 0; j < eNumCom1-1; j++ ) {
//	//		for ( int k = 0; k < eNumCom2; k++ ) {
//	//			printf("%9.6lf ", features[i][j][k]);
//	//		}
//	//		for ( int k = 0; k < eGroup2Num; k++ ) {
//	//			printf("%d ", area_features[i][j][k]);
//	//		}
//	//		puts("");
//	//	}
//	//}
//
//
//	// 離散化テーブルの閾値インデックスの取得
//	for ( int i = 0, j = 0; i < disc->res; i++ ) {
//		if( disc->dat[i] != j ) {
//			bindex[j] = i;
//			j++;
//		}
//	}
//	// 閾値インデックスのアフィン不変量を算出
//	for ( int i = 0; i < disc->num-1; i++ ) {
//		border[i] = bindex[i] * (disc->max-disc->min)/disc->res + disc->min;
//	}
//	int count = 0;
//	int all = 0;
//	int flag = 0;
//	double fxt = 0.0;
//	// 特徴量の得点計算
//	for ( int i = 0; i < p_num; i++ ) {	// 各特徴点
//		for ( int j = 0; j < eNumCom1-1; j++ ) { // 特徴点ごとの特徴量
//			//printf("%d: %d: ", i, j);
//			flag = 0;
//			fx[n] = 0.0;
//			for ( int k = 0; k < eNumCom2; k++ ) {	// 特徴量毎の各次元
//				if ( features[i][j][k] < border[0] ) {					// 離散化領域が左端の場合
//					if ( features[i][j][k] < border[0]/2.0 )	x = 0.0;
//					else										x = (features[i][j][k] / border[0] * 2 - 1.0) * 3.0;
//				}
//				else if ( features[i][j][k] > border[disc->num-2] ) {	// 離散化領域が右端の場合
//					if ( features[i][j][k] > (disc->max + border[disc->num-2])/2.0 )	x = 0.0;
//					else	x = ((features[i][j][k] - border[disc->num-2]) / (disc->max-border[disc->num-2]) * 2 - 1.0)*3.0;
//				} else {
//					for ( int l = 0; l < disc->num-1; l++ ) {
//						if ( features[i][j][k] >= border[l] && features[i][j][k] < border[l+1] ) {
//							x = ((features[i][j][k] - border[l]) / (border[l+1]-border[l]) * 2 - 1.0)*3.0;
//							break;
//						}
//					}
//				}
//
//				// f(x)の計算
//				fxt = exp((double)(-x*x/2));
//				//printf("%9.6lf ", fxt);
//				if ( fxt < 0.02 ) {
//					flag = 1;
//				}
//				// 各次元のf(x)を合算
//				fx[n] += fxt;
//				//printf("%9.6lf ", fxt);
//			}
//			//puts("");
//			if ( flag == 0 ) {
//				//puts("no flag");
//				f_id[n] = j;	// 特徴量ID保存
//				p_id[n] = i;	// 特徴点ID保存
//				//printf("%3d: %2d: %09.6lf\n", p_id[n], f_id[n], fx[n]);
//				n++;
//			}
//		}
//	}
//
//	// 画像を分割し，各領域で保存特徴量数を変える
//	for ( int i = 0; i < tx; i++ ) {
//		for ( int j = 0; j < ty; j++ ) {
//			double *div_fx	= (double *)calloc(div_pnum[i][j]*eNumCom1, sizeof(double));	// 特徴量の得点
//			int *div_pid	= (int *)	calloc(div_pnum[i][j]*eNumCom1, sizeof(int));		// 特徴点ID
//			int *div_fid	= (int *)	calloc(div_pnum[i][j]*eNumCom1, sizeof(int));		// 特徴点内の特徴量ID
//			int m=0;
//			for ( int k = 0; k < div_pnum[i][j]; k++ ) {	// 領域 (i,j)の
//				int id = div_ps[i][j][k];					// k番目の点IDに対して
//				for ( int l = 0; l < n; l++ ) {				// 削除されずに残った特徴量の中に
//					if ( id == p_id[l] ) {					// 点IDが一致するものがあれば
//						div_pid[m] = p_id[l];
//						div_fid[m] = f_id[l];
//						div_fx[m] = fx[l];
//						m++;
//					}
//				}
//			}
//			//for ( int k = 0; k < m; k++ ) 	printf("%d, %d, %lf\n", div_pid[k], div_fid[k], div_fx[k]);
//
//			double temp;
//			int tid;
//			for (int k = 0; k < m - 1; k++) {
//				for (int l = m - 1; l > k; l--) {
//					if (div_fx[l - 1] < div_fx[l]) {  /* 前の要素の方が大きかったら */
//						temp = div_fx[l];        /* 交換する */
//						div_fx[l] = div_fx[l - 1];
//						div_fx[l - 1]= temp;
//
//						tid = div_pid[l];
//						div_pid[l] = div_pid[l - 1];
//						div_pid[l - 1] = tid;
//
//						tid = div_fid[l];
//						div_fid[l] = div_fid[l - 1];
//						div_fid[l - 1] = tid;
//					}
//				}
//			}
//			//for ( int k = 0; k < m; k++ ) 	printf("%d, %d, %lf\n", div_pid[k], div_fid[k], div_fx[k]);
//
//			int f_num = std::min(m,(int)((double)div_pnum[i][j]/(double)p_num*400.0));
//			//printf("%d\n", f_num);
//
//			for ( int k = 0; k < f_num; k++ ) {	// 各特徴点
//
//				for ( int l = 0; l < eNumCom2; l++ ) {
//					if (eTrueHist)	hindex[l] = Affine2Disc2( features[div_pid[k]][div_fid[k]][l], disc );
//					else			hindex[l] = Affine2Disc( features[div_pid[k]][div_fid[k]][l], disc );
//				}
//				// ハッシュ表のインデックスを計算
//				if ( eUseArea ) {	// 面積特徴量追加
//					index = HashFuncArea( hindex, area_features[div_pid[k]][div_fid[k]], disc->num, &quotient );
//				}
//				else {
//					index = HashFunc( hindex, disc->num, &quotient );
//				}
//				//printf("%u\n", index);
//				if ( eCompressHash )	AddHashCompress( index, doc_id, div_pid[k], quotient, hash );
//				else 					AddHash( index, doc_id, div_pid[k], quotient, hash );
//			}
//
//			free( div_fx );
//			free( div_pid );
//			free( div_fid );
//		}
//	}
//
//
//	//// 得点による特徴量のソート
//	//double temp;
//	//int tid;
//	//for (int i = 0; i < n - 1; i++) {
//	//	for (int j = n - 1; j > i; j--) {
//	//		if (fx[j - 1] < fx[j]) {  /* 前の要素の方が大きかったら */
//	//			temp = fx[j];        /* 交換する */
//	//			fx[j] = fx[j - 1];
//	//			fx[j - 1]= temp;
//
//	//			tid = p_id[j];
//	//			p_id[j] = p_id[j - 1];
//	//			p_id[j - 1] = tid;
//
//	//			tid = f_id[j];
//	//			f_id[j] = f_id[j - 1];
//	//			f_id[j - 1] = tid;
//	//		}
//	//	}
//	//}
//
//	////for ( int i = 0; i < 400; i++ ) {
//	////	printf("%3d: %2d: %9.6f\n", p_id[i], f_id[i], fx[i]);
//	////}
//
//	//printf("%d\n", n);
//	//int f_num = std::min(n,400);
//
//	//for ( int i = 0; i < f_num; i++ ) {	// 各特徴点
//	//	for ( int j = 0; j < eNumCom2; j++ ) {
//	//		hindex[j] = Affine2Disc( features[p_id[i]][f_id[i]][j], disc );
//	//	}
//	//	// ハッシュ表のインデックスを計算
//	//	if ( eUseArea ) {	// 面積特徴量追加
//	//		index = HashFuncArea( hindex, area_features[p_id[i]][f_id[i]], disc->num, &quotient );
//	//	}
//	//	else {
//	//		index = HashFunc( hindex, disc->num, &quotient );
//	//	}
//	//	//printf("%u\n", index);
//	//	if ( eCompressHash )	AddHashCompress( index, doc_id, p_id[i], quotient, hash );
//	//	else 					AddHash( index, doc_id, p_id[i], quotient, hash );
//	//}
//
//	free( fx );	// 特徴量の得点
//	free( p_id );	// 特徴量の得点
//	free( bindex );	// 特徴量の得点
//	free( border );	// 特徴量の得点
//	free( hindex );
//}


void RankFeatureAndAddHash2(double ***features, char ***area_features, int doc_id, int p_num, strDisc *disc, double *areas, strHash **hash, int tx, int ty, int ***div_ps, int **div_pnum, int *spare_fnums, char ***spare_features )
{
	double x;
	int n=0;
	unsigned int index;
	unsigned char quotient;

	double *fx	= (double *)calloc(p_num*(eNumCom1-1), sizeof(double));		// 特徴量の得点
	int *p_id		= (int *)	calloc(p_num*(eNumCom1-1), sizeof(int));	// 特徴量の特徴点ID
	int *f_id		= (int *)	calloc(p_num*(eNumCom1-1), sizeof(int));	// 特徴量の特徴点ID
	int *bindex = (int *)calloc(disc->num-1, sizeof(int));					// 離散化テーブルの閾値ID
	double *border = (double *)calloc(disc->num-1, sizeof(double));			// 閾値IDにおける不変量値
	char *hindex = ( char * )calloc( eNumCom2, sizeof(char) );

	//for ( int i = 0; i < p_num; i++ ) {
	//	for ( int j = 0; j < eNumCom1-1; j++ ) {
	//		for ( int k = 0; k < eNumCom2; k++ ) {
	//			printf("%9.6lf ", features[i][j][k]);
	//		}
	//		for ( int k = 0; k < eGroup2Num; k++ ) {
	//			printf("%d ", area_features[i][j][k]);
	//		}
	//		puts("");
	//	}
	//}


	// 離散化テーブルの閾値インデックスの取得
	for ( int i = 0, j = 0; i < disc->res; i++ ) {
		if( disc->dat[i] != j ) {
			bindex[j] = i;
			j++;
		}
	}
	// 閾値インデックスのアフィン不変量を算出
	for ( int i = 0; i < disc->num-1; i++ ) {
		border[i] = bindex[i] * (disc->max-disc->min)/disc->res + disc->min;
	}
	int flag = 0;
	double fxt = 0.0;
	// 特徴量の得点計算
	for ( int i = 0; i < p_num; i++ ) {	// 各特徴点
		for ( int j = 0; j < eNumCom1-1; j++ ) { // 特徴点ごとの特徴量
			//printf("%d: %d: ", i, j);
			flag = 0;
			fx[n] = 0.0;
			for ( int k = 0; k < eNumCom2; k++ ) {	// 特徴量毎の各次元
				if ( features[i][j][k] < border[0] ) {					// 離散化領域が左端の場合
					if ( features[i][j][k] < border[0]/2.0 )	x = 0.0;
					else										x = (features[i][j][k] / border[0] * 2 - 1.0) * 3.0;
				}
				else if ( features[i][j][k] > border[disc->num-2] ) {	// 離散化領域が右端の場合
					if ( features[i][j][k] > (disc->max + border[disc->num-2])/2.0 )	x = 0.0;
					else	x = ((features[i][j][k] - border[disc->num-2]) / (disc->max-border[disc->num-2]) * 2 - 1.0)*3.0;
				} else {
					for ( int l = 0; l < disc->num-1; l++ ) {
						if ( features[i][j][k] >= border[l] && features[i][j][k] < border[l+1] ) {
							x = ((features[i][j][k] - border[l]) / (border[l+1]-border[l]) * 2 - 1.0)*3.0;
							break;
						}
					}
				}

				// f(x)の計算
				fxt = exp((double)(-x*x/2));
				//printf("%9.6lf ", fxt);
				if ( fxt < 0.025 ) {
					flag = 1;
				}
				// 各次元のf(x)を合算
				fx[n] += fxt;
				//printf("%9.6lf ", fxt);
			}
			//puts("");
			if ( flag == 0 ) {
				//puts("no flag");
				f_id[n] = j;	// 特徴量ID保存
				p_id[n] = i;	// 特徴点ID保存
				//printf("%3d: %2d: %09.6lf\n", p_id[n], f_id[n], fx[n]);
				n++;
			}
		}
	}

	double *spare_fx	= (double *)calloc(n, sizeof(double));	// 特徴量の得点
	int    *spare_pid	= (int *)	calloc(n, sizeof(int));		// 特徴量の特徴点ID
	int    *spare_fid	= (int *)	calloc(n, sizeof(int));		// 特徴量の特徴点ID
	int count = 0;

	// 画像を分割し，各領域で保存特徴量数を変える
	for ( int i = 0; i < tx; i++ ) {
		for ( int j = 0; j < ty; j++ ) {
			double *div_fx	= (double *)calloc(div_pnum[i][j]*eNumCom1, sizeof(double));	// 特徴量の得点
			int *div_pid	= (int *)	calloc(div_pnum[i][j]*eNumCom1, sizeof(int));		// 特徴点ID
			int *div_fid	= (int *)	calloc(div_pnum[i][j]*eNumCom1, sizeof(int));		// 特徴点内の特徴量ID
			int m=0;
			for ( int k = 0; k < div_pnum[i][j]; k++ ) {	// 領域 (i,j)の
				int id = div_ps[i][j][k];					// k番目の点IDに対して
				for ( int l = 0; l < n; l++ ) {				// 削除されずに残った特徴量の中に
					if ( id == p_id[l] ) {					// 点IDが一致するものがあれば
						div_pid[m] = p_id[l];
						div_fid[m] = f_id[l];
						div_fx[m] = fx[l];
						m++;
					}
				}
			}
			//for ( int k = 0; k < m; k++ ) 	printf("%d, %d, %lf\n", div_pid[k], div_fid[k], div_fx[k]);

			double temp;
			int tid;
			for (int k = 0; k < m - 1; k++) {
				for (int l = m - 1; l > k; l--) {
					if (div_fx[l - 1] < div_fx[l]) {  /* 前の要素の方が大きかったら */
						temp = div_fx[l];        /* 交換する */
						div_fx[l] = div_fx[l - 1];
						div_fx[l - 1]= temp;

						tid = div_pid[l];
						div_pid[l] = div_pid[l - 1];
						div_pid[l - 1] = tid;

						tid = div_fid[l];
						div_fid[l] = div_fid[l - 1];
						div_fid[l - 1] = tid;
					}
				}
			}
			//for ( int k = 0; k < m; k++ ) 	printf("%d, %d, %lf\n", div_pid[k], div_fid[k], div_fx[k]);

			int f_num = std::min( m, (int)((double)div_pnum[i][j]/(double)p_num*400.0));
			//printf("%d\n", f_num);

			for ( int k = 0; k < f_num; k++ ) {	// 各特徴点

				for ( int l = 0; l < eNumCom2; l++ ) {
					hindex[l] = Affine2Disc( features[div_pid[k]][div_fid[k]][l], disc );
				}
				// ハッシュ表のインデックスを計算
				if ( eUseArea ) {	// 面積特徴量追加
					index = HashFuncArea( hindex, area_features[div_pid[k]][div_fid[k]], disc->num, &quotient );
				}
				else {
					index = HashFunc( hindex, disc->num, &quotient );
				}
				//printf("%u\n", index);
				if ( eCompressHash )	AddHashCompress( index, doc_id, div_pid[k], quotient, hash );
				else 					AddHash( index, doc_id, div_pid[k], quotient, hash );
			}

			// スペアを保存
			for ( int k = f_num; k < m; k++ ) {
				spare_pid[count] = div_pid[k];
				spare_fid[count] = div_fid[k];
				spare_fx[count] = div_fx[k];
				count++;
			}

			free( div_fx );
			free( div_pid );
			free( div_fid );
		}
	}

	double temp;
	int tid;
	for (int i = 0; i < count - 1; i++) {
		for (int j = count - 1; j > i; j--) {
			if (spare_fx[j - 1] < spare_fx[j]) {  /* 前の要素の方が大きかったら */
				temp = spare_fx[j];        /* 交換する */
				spare_fx[j] = spare_fx[j - 1];
				spare_fx[j - 1]= temp;

				tid = spare_pid[j];
				spare_pid[j] = spare_pid[j - 1];
				spare_pid[j - 1] = tid;

				tid = spare_fid[j];
				spare_fid[j] = spare_fid[j - 1];
				spare_fid[j - 1] = tid;
			}
		}
	}
	//for ( int i = 0; i < count; i++ ) 	printf("%d, %d, %lf\n", spare_pid[i], spare_fid[i], spare_fx[i]);

	spare_fnums[doc_id] = count;
	spare_features[doc_id] = (char **)calloc(count, sizeof(char *));
	for ( int i = 0; i < count; i++ ) {
		spare_features[doc_id][i] = (char *)calloc(sizeof(char)+sizeof(unsigned int)+sizeof(int)+sizeof(char)+sizeof(unsigned int), sizeof(char));
	}

	for ( int i = 0; i < count; i++ ) {
		for ( int j = 0; j < eNumCom2; j++ ) {
			hindex[j] = Affine2Disc( features[spare_pid[i]][spare_fid[i]][j], disc );
		}
		// ハッシュ表のインデックスを計算
		if ( eUseArea ) {	// 面積特徴量追加
			index = HashFuncArea( hindex, area_features[spare_pid[i]][spare_fid[i]], disc->num, &quotient );
		}
		else {
			index = HashFunc( hindex, disc->num, &quotient );
		}
		//printf("%u\n", index);
		//printf("%4d, %d, %9.6lf, %10u\n", spare_pid[i], spare_fid[i], spare_fx[i], index);
		spare_features[doc_id][i][0] = 0;
		int buff_cor = 1;
		memcpy(&spare_features[doc_id][i][buff_cor], &index, sizeof(unsigned int));
		buff_cor += sizeof(unsigned int);
		memcpy(&spare_features[doc_id][i][buff_cor], &spare_pid[i], sizeof(int));
		buff_cor += sizeof(int);
		memcpy(&spare_features[doc_id][i][buff_cor], &quotient, sizeof(unsigned char));
		buff_cor += sizeof(unsigned char);
		memcpy(&spare_features[doc_id][i][buff_cor], &doc_id, sizeof(int));

	}

	free( fx );	// 特徴量の得点
	free( p_id );	// 特徴量の得点
	free( bindex );	// 特徴量の得点
	free( border );	// 特徴量の得点
	free( hindex );
	free( spare_fx );
	free( spare_pid );
	free( spare_fid );
}

void RankFeatureAndAddHash3(double ***features, char ***area_features, int doc_id, int p_num, strDisc *disc, double *areas, strHash **hash, int tx, int ty, int ***div_ps, int **div_pnum)
{
	double x;
	int n=0;
	unsigned int index;
	unsigned char quotient;

	double *fx	= (double *)calloc(p_num*(eNumCom1-1), sizeof(double));		// 特徴量の得点
	int *p_id		= (int *)	calloc(p_num*(eNumCom1-1), sizeof(int));	// 特徴量の特徴点ID
	int *f_id		= (int *)	calloc(p_num*(eNumCom1-1), sizeof(int));	// 特徴量の特徴点ID
	int *bindex = (int *)calloc(disc->num-1, sizeof(int));					// 離散化テーブルの閾値ID
	double *border = (double *)calloc(disc->num-1, sizeof(double));			// 閾値IDにおける不変量値
	char *hindex = ( char * )calloc( eNumCom2, sizeof(char) );

	// 離散化テーブルの閾値インデックスの取得
	for ( int i = 0, j = 0; i < disc->res; i++ ) {
		if( disc->dat[i] != j ) {
			bindex[j] = i;
			j++;
		}
	}
	// 閾値インデックスのアフィン不変量を算出
	for ( int i = 0; i < disc->num-1; i++ ) {
		border[i] = bindex[i] * (disc->max-disc->min)/disc->res + disc->min;
	}
	int count = 0;
	int all = 0;
	int flag = 0;
	double fxt = 0.0;
	// 特徴量の得点計算
	for ( int i = 0; i < p_num; i++ ) {	// 各特徴点
		for ( int j = 0; j < eNumCom1-1; j++ ) { // 特徴点ごとの特徴量
			//printf("%d: %d: ", i, j);
			flag = 0;
			fx[n] = 0.0;
			for ( int k = 0; k < eNumCom2; k++ ) {	// 特徴量毎の各次元
				if ( features[i][j][k] < border[0] ) {					// 離散化領域が左端の場合
					if ( features[i][j][k] < border[0]/2.0 )	x = 0.0;
					else										x = (features[i][j][k] / border[0] * 2 - 1.0) * 3.0;
				}
				else if ( features[i][j][k] > border[disc->num-2] ) {	// 離散化領域が右端の場合
					if ( features[i][j][k] > (disc->max + border[disc->num-2])/2.0 )	x = 0.0;
					else	x = ((features[i][j][k] - border[disc->num-2]) / (disc->max-border[disc->num-2]) * 2 - 1.0)*3.0;
				} else {
					for ( int l = 0; l < disc->num-1; l++ ) {
						if ( features[i][j][k] >= border[l] && features[i][j][k] < border[l+1] ) {
							x = ((features[i][j][k] - border[l]) / (border[l+1]-border[l]) * 2 - 1.0)*3.0;
							break;
						}
					}
				}

				// f(x)の計算
				fxt = exp((double)(-x*x/2));
				//printf("%9.6lf ", fxt);
				if ( fxt < 0.02 ) {
					flag += 1;
				}
				// 各次元のf(x)を合算
				fx[n] += fxt;
				//printf("%9.6lf ", fxt);
			}
			//puts("");
			if ( flag < 2 ) {
				//puts("no flag");
				f_id[n] = j;	// 特徴量ID保存
				p_id[n] = i;	// 特徴点ID保存
				//printf("%3d: %2d: %09.6lf\n", p_id[n], f_id[n], fx[n]);
				n++;
			}
		}
	}

	// 画像を分割し，各領域で保存特徴量数を変える
	for ( int i = 0; i < tx; i++ ) {
		for ( int j = 0; j < ty; j++ ) {
			double *div_fx	= (double *)calloc(div_pnum[i][j]*eNumCom1, sizeof(double));	// 特徴量の得点
			int *div_pid	= (int *)	calloc(div_pnum[i][j]*eNumCom1, sizeof(int));		// 特徴点ID
			int *div_fid	= (int *)	calloc(div_pnum[i][j]*eNumCom1, sizeof(int));		// 特徴点内の特徴量ID
			int m=0;
			for ( int k = 0; k < div_pnum[i][j]; k++ ) {	// 領域 (i,j)の
				int id = div_ps[i][j][k];					// k番目の点IDに対して
				for ( int l = 0; l < n; l++ ) {				// 削除されずに残った特徴量の中に
					if ( id == p_id[l] ) {					// 点IDが一致するものがあれば
						div_pid[m] = p_id[l];
						div_fid[m] = f_id[l];
						div_fx[m] = fx[l];
						m++;
					}
				}
			}
			//for ( int k = 0; k < m; k++ ) 	printf("%d, %d, %lf\n", div_pid[k], div_fid[k], div_fx[k]);

			double temp;
			int tid;
			for (int k = 0; k < m - 1; k++) {
				for (int l = m - 1; l > k; l--) {
					if (div_fx[l - 1] < div_fx[l]) {  /* 前の要素の方が大きかったら */
						temp = div_fx[l];        /* 交換する */
						div_fx[l] = div_fx[l - 1];
						div_fx[l - 1]= temp;

						tid = div_pid[l];
						div_pid[l] = div_pid[l - 1];
						div_pid[l - 1] = tid;

						tid = div_fid[l];
						div_fid[l] = div_fid[l - 1];
						div_fid[l - 1] = tid;
					}
				}
			}
			//for ( int k = 0; k < m; k++ ) 	printf("%d, %d, %lf\n", div_pid[k], div_fid[k], div_fx[k]);

			int f_num = std::min(m,(int)((double)div_pnum[i][j]/(double)p_num*400.0));
			//printf("%d\n", f_num);

			for ( int k = 0; k < f_num; k++ ) {	// 各特徴点

				for ( int l = 0; l < eNumCom2; l++ ) {
					if (eTrueHist)	hindex[l] = Affine2Disc2( features[div_pid[k]][div_fid[k]][l], disc );
					else			hindex[l] = Affine2Disc( features[div_pid[k]][div_fid[k]][l], disc );
				}
				// ハッシュ表のインデックスを計算
				if ( eUseArea ) {	// 面積特徴量追加
					index = HashFuncArea( hindex, area_features[div_pid[k]][div_fid[k]], disc->num, &quotient );
				}
				else {
					index = HashFunc( hindex, disc->num, &quotient );
				}
				//printf("%u\n", index);
				if ( eCompressHash )	AddHashCompress( index, doc_id, div_pid[k], quotient, hash );
				else 					AddHash( index, doc_id, div_pid[k], quotient, hash );
			}

			free( div_fx );
			free( div_pid );
			free( div_fid );
		}
	}

	free( fx );	// 特徴量の得点
	free( p_id );	// 特徴量の得点
	free( bindex );	// 特徴量の得点
	free( border );	// 特徴量の得点
	free( hindex );
}
