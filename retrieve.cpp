#include "def_general.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <opencv2/core/core.hpp>

#ifdef WIN32
#ifdef _DEBUG
#pragma comment( lib, "opencv_core231d.lib" )
#else
#pragma comment( lib, "opencv_core231.lib" )
#endif
#endif

#include "extern.h"
#include "proctime.h"
#include "affine.h"
#include "hcompress.h"
#include "disc.h"
#include "hash.h"
#include "fpath.h"

extern strHash cut;
int corres_num=0;
int corres[3][1000 * kMaxPointNum];
std::vector<unsigned int> vote;
int fnum=0;
int mnum=0;
int dnum=0;

using namespace std;

int VoteDirectlyByHL( strHash *hl, int *score, int p, unsigned char quotient )
// ハッシュリストから直にvoteする
// 今回voteした中で最大のものを返す
// 特徴量の比較を，商の比較に変更
{
	int i;
	static int max_vote = -1;
	int hdoc, max_doc=0;
	short hpoint;
	unsigned char hquotient;

	if( hl != NULL ) {
		for( i = 0; i < *hl; i++ ) {
			hdoc		= *( ( int *			)( hl + 1 + i * 7		) );
			hpoint		= *( ( short *			)( hl + 1 + i * 7 + 4	) );
			hquotient	= *( ( unsigned char *	)( hl + 1 + i * 7 + 6	) );

			if ( //flag1[hdoc] == 0 && /* 対応点フラグチェック */ 
				 //flag2[hdoc][hpoint] == 0 && 
				 ( memcmp(&hquotient, &quotient, 1) == 0 ) /* 商の比較 */ ) {
				score[hdoc]++;	// 投票
				if ( score[hdoc] > max_vote ) {
					max_vote = score[hdoc];	// 最大なら更新
					max_doc = hdoc;
				}

				//flag1[hdoc] = 1;	// hp->docは投票済み
				//flag2[hdoc][hpoint] = 1;	// hp-docのhp->pointは投票済み
				//flag1_clear[flag1_num++] = hdoc;
				//flag2_dclear[flag2_num] = hdoc;
				//flag2_pclear[flag2_num] = hpoint;
				//corres[0][flag2_num] = hdoc;
				//corres[1][flag2_num] = p;
				//corres[2][flag2_num++] = hpoint;

				//if ( max_vote > 10 && hdoc == max_doc )
				//	break;
			}
		}
	}

	return max_vote;
}

int VoteDirectlyByHL2( strHash *hl, int *score, int p, unsigned char quotient )
// ハッシュリストから直にvoteする
// 今回voteした中で最大のものを返す
// 特徴量の比較を，商の比較に変更
{
	int i;
	static int max_vote = -1;
	unsigned int pdoc, max_doc=0;
	unsigned short ppoint;
	unsigned char pquotient;
	unsigned char mask = 255 >> (8-eQuotBit);

	if( hl != NULL && hl != &cut ) {
		for( i = 0; i < *hl; i++ ) {
			
			ReadHComp2Dat( (hl+1+eHComp2DatByte*i), &pdoc, &ppoint, &pquotient );
			//quotient = quotient & 0x0007;
			quotient = quotient & mask;
			//printf("%d : %d : %d : %d\n", pdoc, ppoint, pquotient, quotient);
			//pdoc -= 5000;
			if ( //flag1[pdoc] == 0 && 
				 //flag2[pdoc][ppoint] == 0 &&
				 //pdoc != 0 &&
				 ( memcmp(&pquotient, &quotient, 1) == 0 )) {
				score[pdoc]++;	// 投票
				vote.push_back(pdoc);

				if ( score[pdoc] > max_vote ) {
					max_vote = score[pdoc];	// 最大なら更新
					max_doc = pdoc;
				}

				//flag1[pdoc] = 1;	// hp->docは投票済み
				//flag2[pdoc][ppoint] = 1;	// hp-docのhp->pointは投票済み
				//flag1_clear[flag1_num++] = pdoc;
				//flag2_dclear[flag2_num] = pdoc;
				//flag2_pclear[flag2_num] = ppoint;
				corres[0][corres_num] = pdoc;
				corres[1][corres_num] = p;
				corres[2][corres_num++] = ppoint;

				//if ( max_vote > 10 && pdoc == max_doc )
				//	break;
			}
		}
	}

	return max_vote;
}

void CalcScoreRotateOnce( CvPoint *ps, double *areas, int num, int *score, strDisc *disc, strHash **hash )
// アフィン不変量を主成分得点に変換してハッシュに登録する
// 回転不変版
{
	char **hindex = NULL, *hindex_area = NULL, *inv_array = NULL, *vec_flag=NULL, *sum=NULL;
	int i, j, k, l, m, st, row, max_vote = 0;
	int *idx = NULL, *idxcom1 = NULL, *idxcom2 = NULL, *idxcom1bak = NULL;
	unsigned long long int index;
	unsigned char quotient;
	double cr;

	// 配列の確保
	idx = (int *)calloc( eGroup1Num, sizeof(int) );
	idxcom1 = (int *)calloc( eGroup2Num, sizeof(int) );
	idxcom1bak = (int *)calloc( eGroup2Num-1, sizeof(int) );
	idxcom2 = (int *)calloc( eGroup3Num, sizeof(int) );
	inv_array = (char *)calloc( eGroup2Num-1, sizeof(char) );
	hindex_area = (char *)calloc( eGroup2Num, sizeof(char) );
	hindex = (char **)calloc( eMultiVecNum, sizeof(char *) );
	for ( i = 0; i < eMultiVecNum; i++ )
		hindex[i] = (char *)calloc( eNumCom2, sizeof(char) );
	if ( eRetMultiVec ) {
		vec_flag = (char *)calloc( eNumCom2, sizeof(char) );
		sum = (char *)calloc( eNumCom2, sizeof(char) );
	}
	//memset( score, 0, sizeof( int ) * eDbDocs );

	for ( i = 0; i < num; i++ ) {	// for all points
		//ClearFlag1();
		// 近傍店を時計回りに整列
		CalcOrderCWN( i, ps, idx, eGroup1Num );

		for ( j = 0; j < eNumCom1-1; j++ )	{	// nCm
			for ( k = 0; k < eGroup2Num; k++ )	idxcom1[k] = idx[com1[j][k]];	// m点の抽出
			/************** 開始点の探索 **************************************************/
			
			// 不変量の計算
			for ( st = 0; st < eGroup2Num-1; st++ ) {
				cr = CalcAffineInv( ps[idxcom1[0]], ps[idxcom1[(st%(eGroup2Num-1))+1]], ps[idxcom1[((st+1)%(eGroup2Num-1))+1]], ps[idxcom1[((st+2)%(eGroup2Num-1))+1]]);
				if ( eTrueHist )	inv_array[st] = Affine2Disc2( cr, disc );
				else				inv_array[st] = Affine2Disc( cr, disc );
			}
			
			// 不変量から開始点を見つける
			st = FindStartPoint( inv_array, eGroup2Num-1 );

			// stを開始点として回転させる
			for ( k = 0; k < eGroup2Num-1; k++ )	idxcom1bak[k] = idxcom1[k+1];
			for ( k = 0; k < eGroup2Num-1; k++ )	idxcom1[k+1] = idxcom1bak[(k+st)%(eGroup2Num-1)];
			/************** 探索の終了 ****************************************************/

			// アフィン不変量を標準化
			for ( k = 0; k < eNumCom2; k++ ) {

				for ( l = 0; l < eGroup3Num; l++ )	idxcom2[l] = idxcom1[com2[k][l]];
				cr = CalcAffineInv(ps[idxcom2[0]], ps[idxcom2[1]], ps[idxcom2[2]], ps[idxcom2[3]]);
				//if ( cr > 10.0 )	cr = 10.0;

				if ( eRetMultiVec )	hindex[0][k] = Affine2DiscForRetrieve( cr, disc, &vec_flag[k] );
				else {
					if ( eTrueHist )	hindex[0][k] = Affine2Disc2( cr, disc );
					else				hindex[0][k] = Affine2Disc( cr, disc );
				}
			}
			//for ( k = 0; k < 17; k++ )	printf("%2d", vec_flag[k]);
			//puts("");
			//for ( k = 0; k < 17; k++ )	printf("%d ", hindex[0][k]);
			//puts("");

			if ( eUseArea ) CalcHindexAreaRatio( idxcom1, areas, hindex_area );

			if ( eRetMultiVec ) {
				row = 1;
				for ( k = 0; k < eMultiNum; k++ ) {
				//for ( k = 0; k < eNumCom2; k++ ) {
					if ( vec_flag[k] != 0 ) {
						sum[k] = vec_flag[k];
						for ( l = 0; l < row; l++ ) {
							for ( m = 0; m < eNumCom2; m++ ) {
								hindex[row + l][m] = hindex[l][m] + sum[m];
							}
						}
						row *= 2;
						sum[k] = 0;
					}
				}
				//printf( "%d\n", row );
				//for ( k = 0; k < row; k++ ) {
				//	for ( l = 0; l < 17; l++ ) printf("%d ", hindex[k][l]);
				//	puts("");
				//}
				fnum+=row;
				mnum++;
				for ( k = 0; k < row; k++ ) {
					//for ( l = 0; l < 17; l++ ) printf("%d ", hindex[k][l]);
					//puts("");
					if ( eUseArea )	index = HashFuncArea( hindex[k], hindex_area, disc->num, &quotient );
					else			index = HashFunc( hindex[k], disc->num, &quotient );
					//printf("%d\n", index);
					if ( eCompressHash )	max_vote = VoteDirectlyByHL2( hash[index], score, i, quotient );
					else					max_vote = VoteDirectlyByHL( hash[index], score, i, quotient );
				}
				memset( vec_flag, 0, sizeof( char ) * eNumCom2 );
			} else {
				if ( eUseArea )	index = HashFuncArea( hindex[0], hindex_area, disc->num, &quotient );
				else			index = HashFunc( hindex[0], disc->num, &quotient );
				//printf("%llu\n", index);
				if ( eCompressHash )	max_vote = VoteDirectlyByHL2( hash[index], score, i, quotient );
				else					max_vote = VoteDirectlyByHL( hash[index], score, i, quotient );
			}
		}
	}
	
	dnum++;
	//cout << dnum << ":" << mnum << ":" << fnum << endl;

	for ( i = 0; i < eMultiVecNum; i++ )	free( hindex[i] );
	free( hindex );
	free( hindex_area );
	free( idx );
	free( idxcom1 );
	free( idxcom2 );
	free( idxcom1bak );
	free( inv_array );
	free( vec_flag );
	free( sum );
}

int Retrieve( CvPoint *ps, double *areas, int num, int *score, strDisc *disc, int *reg_nums, int *ret_time, strHash **hash )
// ハッシュから検索する
{
	int i, max_doc = 0;
	int  max_score = 0;
	int start, end;

	//// flag2の初期化
	//ClearFlag2();
	//// 点対応テーブルの作成
	//ClearCorres();

	// 検索開始
	start = GetProcTimeMicroSec();
	// 検索
	std::sort(vote.begin(), vote.end());
	vote.erase( std::unique(vote.begin(), vote.end()), vote.end());
	for ( int i = 0; i < vote.size(); i++ ) {
		score[vote[i]] = 0;
	}
	vote.clear();

	//memset( score, 0, sizeof( int ) * eDbDocs );
	CalcScoreRotateOnce( ps, areas, num, score, disc, hash );
	// 検索終了
	end = GetProcTimeMicroSec();



	// 検索時間
	*ret_time = end - start;
	if ( !eExperimentMode )	printf( "retrieval time : %d[milisec]\n", *ret_time );
	
	// ソート
	for ( i = 0; i < eDbDocs; i++ ) {
		if ( score[i] > max_score ) {
			max_doc = i;
			max_score = score[i];
		}
	}

	return max_doc;
}

int RetrieveCor( CvPoint *ps, double *areas, int num, int *score, int pcor[][2], int *pcornum0, strDisc *disc, int *reg_nums, strHash **hash )
// ハッシュから検索する
{
	int i, j, k, max_doc=0, flag;
	int  max_score = 0;

	memset( corres[0], -1, sizeof(int)*corres_num );
	corres_num = 0;

	// 検索
	CalcScoreRotateOnce( ps, areas, num, score, disc, hash );

	// スコアの正規化
	for ( i = 0; i < eDbDocs; i++ )	{
		score[i] = (int)((double)score[i] - (double)reg_nums[i] * eProp);	// CBDARでの補正
	}

	// ソート
	for ( i = 0; i < eDbDocs; i++ ) {
		if ( score[i] > max_score ) {
			max_doc = i;
			max_score = score[i];
		}
	}
	for ( i = 0, j = 0; i < corres_num; i++ ) {
		flag = 0;
		if ( corres[0][i] == max_doc )	{
			for ( k = 0; k < j; k++ ) {
				if ( pcor[k][0] == corres[1][i] ) flag = 1;
			}
			if ( flag != 1 ) {
				pcor[j][0] = corres[1][i];
				pcor[j][1] = corres[2][i];
//			fprintf( fp1, "%d,%d\n", pcor[j][0], pcor[j][1] );
				//printf( "%d,%d\n", pcor[j][0], pcor[j][1] );
				j++;
			}
		}
	}
	*pcornum0 = j;

	return max_doc;
}

int IsSucceed( char *str1, char *str2 )
// 実験モード用：成功判定
{
	char base1[kMaxPathLen], base2[kMaxPathLen];
	
	GetBasename( str1, kMaxPathLen, base1 );
	GetBasename( str2, kMaxPathLen, base2 );

	return !strcmp( base1, base2 );
}

double Calc12Diff( int *score )
// 1位と2位の差を計算
{
	int i;
	int score_1st = 1, score_2nd = 1;

	for ( i = 0; i < eDbDocs; i++ ) {
		if ( score[i] > score_1st ) {
			score_2nd = score_1st;
			score_1st = score[i];
		} else if ( score[i] > score_2nd ) {
			score_2nd = score[i];
		}
	}
	return ((double) score_1st) / ((double) score_2nd);
}