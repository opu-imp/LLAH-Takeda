#include "def_general.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <opencv2/core/core.hpp>

#ifdef WIN32
#ifdef _DEBUG
#pragma comment( lib, "opencv_core231d.lib" )
#else
#pragma comment( lib, "opencv_core231.lib" )
#endif
#endif

#include "extern.h"
#include "nears.h"

#define kNears 7
#define kOutNears 10
#define kMinAreaNears 6

#define	GetPointsDistance(p1, p2)	(sqrt((double)((p1.x - p2.x)*(p1.x - p2.x)+(p1.y - p2.y)*(p1.y - p2.y))))

using namespace std;

int *_nears;
int **nears;

void SecureNears( void )
// 近傍構造配列の確保
{
	int i;
	_nears = (int *)malloc( sizeof(int)*(kNears+kOutNears)*kMaxPointNum );
	nears = (int **)malloc( sizeof(int *)*kMaxPointNum );
	for( i = 0; i < kMaxPointNum; i++ ) {
		nears[i] = _nears + i * (kNears+kOutNears);
	}
	memset( _nears, 0, sizeof(int)*(kNears+kOutNears)*kMaxPointNum );
}

void ClearNears( int num )
// 近傍構造配列のクリア
{
	memset( _nears, 0, sizeof(int)*(kNears+kOutNears)*num );
}

void ReleaseNears( void )
// 希望構造配列の解放
{
	free( nears );
	free( _nears );
}

void NearestPoint( int n, CvPoint *ps, int num, int nears0[] )
//	サイズnumの点配列psのn番目の点と最も近いkNears個の点のインデックスをnearsに格納する
// '10/11/10 中心点を含む処理を追加
{
	int i, j, k, min_idxs[kNears+kOutNears];
	double dist, min_dists[kNears+kOutNears];
	CvPoint p;

	// 初期化
	for ( i = 0; i < kNears+kOutNears; i++ ) {
		min_idxs[i] = 0;
		min_dists[i] = 10000000;
	}
	p.x = ps[n].x;
	p.y = ps[n].y;

	for ( i = 0; i < num; i++ ) {
		if ( i == n )	continue;
		dist = sqrt( (double)(( p.x - ps[i].x )*( p.x - ps[i].x )+( p.y - ps[i].y )*( p.y - ps[i].y )) );
		for ( j = 0; j < kNears+kOutNears; j++ ) {
			if ( dist < min_dists[j] ) {
				for ( k = kNears+kOutNears - 1; k > j; k-- ) {
					min_idxs[k] = min_idxs[k - 1];
					min_dists[k] = min_dists[k - 1];
				}
				min_idxs[j] = i;
				min_dists[j] = dist;
				break;
			}
		}
	}

	nears0[0] = n;
	for ( i = 1; i < kNears+kOutNears; i++ ) {
		nears0[i] = min_idxs[i-1];
	}
}

void MakeNearsFromCentres( CvPoint *ps, int num )
// 重心から近傍構造を計算する
{
	int i;

	for ( i = 0; i < num; i++ ) {
		NearestPoint( i, ps, num, nears[i] );
	}
}


int SearchMinNears( CvPoint *ps, double *areas, int num, int *reg_thinids )
// 面積極小地の探索
{
	int *flag = NULL;
	int k, flag2, count = 0, n=0, num1=0;
	double *dist;
	dist = (double *)calloc( num, sizeof(double) );

	//reg_thinids = (int *)calloc(num, sizeof(int));
	for ( int i = 0; i < num; i++ ) {
		reg_thinids[i] = i;
	}

	for ( int i = 0; i < num; i++ ) {
		dist[i] = sqrt( (double)(ps[i].x - ps[nears[i][eGroup1Num]].x)*(ps[i].x - ps[nears[i][eGroup1Num]].x) + (double)(ps[i].y - ps[nears[i][eGroup1Num]].y)*(ps[i].y - ps[nears[i][eGroup1Num]].y) );
		//cout << dist[i] << endl;
	}

	int tmp_id;
	double tmp;
	for ( int i = 0; i < num - 1; i++ ) {
		for ( int j = num - 1; j > i; j-- ) {
			if ( dist[j-1] < dist[j] ) {
				tmp = dist[j];
				dist[j] = dist[j-1];
				dist[j-1] = tmp;

				tmp_id = reg_thinids[j];
				reg_thinids[j] = reg_thinids[j-1];
				reg_thinids[j-1] = tmp_id;
			}
		}
	}

	//for ( int i = 0; i < num - 1; i++ ) {
	//	cout << reg_thinids[i] << ": " << dist[i] << endl;
	//}

	return std::min(num, 400);
}


int SearchMinAreas( CvPoint *ps, double *areas, int num, int *reg_thinids )
// 面積極小地の探索
{
	int *flag = NULL;
	int i, j, k, flag2, count = 0, n=0, num1=0;
	
	if ( num < eThinNum ) {
		for ( i = 0; i < num; i++ )	reg_thinids[i] = i;
		return num;
	}
	else {
		flag = (int *)calloc(num, sizeof(int));
	
		for ( i = 0, count = 0; i < num; i++ ) {
			flag2 = 0;
			for ( j = 0; j < kMinAreaNears; j++ ) {
				if ( areas[i] / areas[nears[i][j+1]] < 1.1)
					flag2++;
			}
			if ( flag2 == kMinAreaNears ) {
				if ( flag[i] == 0 ) {
					reg_thinids[count++] = i;
					flag[i] = 1;
				}
			}
		}
		n = (int)floor(((double)(eThinNum-count)/count) + 0.5);
		for ( i = 0; i < count; i++ ) {
			k=0;
			for ( j = 0; j < n; j++ ) {
				if ( flag[nears[reg_thinids[i]][k+1]] == 0 ) {
					reg_thinids[count+num1] = nears[reg_thinids[i]][k+1];
					flag[nears[reg_thinids[i]][k+1]] = 1;
					num1++;
				}else{
					j--;
				}
				k++;
				if ( k >= kNears )	break;
			}
		}

		free( flag );
		return count+num1;
	}
}

int CompareDiv( const strDiv *div1, const strDiv *div2 )
{
	if ( div1->dist > div2->dist )	return 1;
	else if ( div1->dist < div2->dist )	return -1;
	else	return 0;
}

int ComparePointDist( const strPointDist *pd1, const strPointDist *pd2 )
{
	if ( pd1->dist > pd2->dist )	return 1;
	else if ( pd1->dist < pd2->dist)	return -1;
	else	return 0;
}

double CalcDivDist( CvPoint pt1, CvSize *size, int tx, int ty, int x, int y, int div_x, int div_y )
// 特徴点と分割領域の最短距離を求める
// pt1: 特徴点, size: 画像のサイズへのポインタ, tx: x軸の分割数, ty: y軸の分割数, x&y: 領域, div_x&div_y: 特徴点の属する領域
{
	int left, right, top, bottom;
	CvPoint pt2;
	
	if ( x == div_x && y == div_y )	return 0.0L;	// 領域に含まれる
	// 四隅の座標を求める
	left = (int)(((double)size->width/(double)tx)*x);
	right = (int)(((double)size->width/(double)tx)*(x+1))-1;
	top = (int)(((double)size->height/(double)ty)*y);
	bottom = (int)(((double)size->height/(double)ty)*(y+1))-1;
	// 領域の最近傍点を求める
	if ( x < div_x )		pt2.x = right;
	else if ( x > div_x )	pt2.x = left;
	else					pt2.x = pt1.x;
	if ( y < div_y )		pt2.y = bottom;
	else if ( y > div_y )	pt2.y = top;
	else					pt2.y = pt1.y;
	
	return GetPointsDistance( pt1, pt2 );
}

void MergePointDist( strPointDist *tmp0, int t0num, strPointDist *tmp1, int t1num, int kn, strPointDist **merge0, int *mnum0 )
{
	int i, i0, i1;
	strPointDist *merge;
	
	merge = (strPointDist *)calloc( kn, sizeof(strPointDist) );
	
	for ( i = 0, i0 = 0, i1 = 0; i < kn && ( i0 < t0num || i1 < t1num); i++ ) {
		if ( i1 >= t1num ) {	// tmp1がオーバー
			merge[i].pindex = tmp0[i0].pindex;
			merge[i].dist = tmp0[i0].dist;
			i0++;
		} else if ( i0 >= t0num ) {	// tmp0がオーバー
			merge[i].pindex = tmp1[i1].pindex;
			merge[i].dist = tmp1[i1].dist;
			i1++;
		} else if ( tmp0[i0].dist <= tmp1[i1].dist ) {	// tmp0の方が近い
			merge[i].pindex = tmp0[i0].pindex;
			merge[i].dist = tmp0[i0].dist;
			i0++;
		}
		else {	// tmp1の方が近い
			merge[i].pindex = tmp1[i1].pindex;
			merge[i].dist = tmp1[i1].dist;
			i1++;
		}
	}
	
	*merge0 = merge;
	*mnum0 = i;
}

void MakeNearsFromCentresDiv( CvPoint *ps, int pnum, CvSize *size, int tx, int ty, int kn )
// 重心から近傍構造を計算する（領域分割バージョン）
{
	int i, j, div_x, div_y, **div_pnum, ***div_ps, pi, divi, t0num, t1num, mnum;
	double max_dist;
	strDiv *div;
	strPointDist *tmp0=NULL, *tmp1=NULL, *merge=NULL;

	max_dist = sqrt( (double)(size->width * size->width + size->height * size->height ) );	// 最大距離
	// nearsの領域を確保
	//nears = (int **)calloc(pnum, sizeof(int*));
	//*nears0 = nears;
	//for ( i = 0; i < pnum; i++ ) {
	//	nears[i] = (int *)calloc(kn, sizeof(int));
	//}

	div_pnum = (int **)calloc( tx, sizeof(int*) );	// 各領域の特徴点数
	for ( i = 0; i < tx; i++ ) {
		div_pnum[i] = (int *)calloc( ty, sizeof(int) );
	}
	div_ps = (int ***)calloc( tx, sizeof(int*) );	// 各領域の特徴点
	for ( i = 0; i < tx; i++ ) {
		div_ps[i] = (int **)calloc( ty, sizeof(int*) );
		for ( j = 0; j < ty; j++ ) {
			div_ps[i][j] = (int *)calloc( pnum, sizeof(int) );
		}
	}
	
	for ( i = 0; i < pnum; i++ ) {	// 特徴点の領域への分類
		div_x = (int)(((double)ps[i].x/(double)size->width)*tx);
		div_y = (int)(((double)ps[i].y/(double)size->height)*ty);
		div_ps[div_x][div_y][div_pnum[div_x][div_y]++] = i;
	}
//	DrawDivPoints( ps, div_ps, div_pnum, tx, ty, size, "div_ps.bmp" );
	div = (strDiv *)calloc( tx*ty, sizeof(strDiv) );
	for ( pi = 0; pi < pnum; pi++ ) {	// 各特徴点について
		// 属する領域を求める
		div_x = (int)(((double)ps[pi].x/(double)size->width)*tx);
		div_y = (int)(((double)ps[pi].y/(double)size->height)*ty);
		// 各領域との最短距離を求める
		for ( i = 0; i < tx; i++ ) {
			for ( j = 0; j < ty; j++ ) {
				divi = j*tx+i;
				div[divi].xindex = i;
				div[divi].yindex = j;
				div[divi].dist = CalcDivDist( ps[pi], size, tx, ty, i, j, div_x, div_y );
			}
		}
		// 距離の小さい順にソート
		qsort( div, tx*ty, sizeof(strDiv), (int (*)(const void*, const void *))CompareDiv );
		// tmp0を作成（ダミー）
		tmp0 = (strPointDist *)calloc( 1, sizeof(strPointDist) );
		t0num = 0;
		for ( divi = 0; divi < tx*ty; divi++ ) {	// 近い領域から処理
			// 領域diviの特徴点と距離を得る
			div_x = div[divi].xindex;
			div_y = div[divi].yindex;
			t1num = div_pnum[div_x][div_y];
			tmp1 = (strPointDist *)calloc( t1num, sizeof(strPointDist) );
			for ( i = 0; i < t1num; i++ ) {
				tmp1[i].pindex = div_ps[div_x][div_y][i];
				if ( tmp1[i].pindex == pi )	tmp1[i].dist = max_dist;
				else	tmp1[i].dist = GetPointsDistance( ps[pi], ps[tmp1[i].pindex] );
			}
			// 近い順にソートする
			qsort( tmp1, t1num, sizeof(strPointDist), (int (*)(const void *, const void *))ComparePointDist );
			// マージする
			MergePointDist( tmp0, t0num, tmp1, t1num, kn, &merge, &mnum );
			// kn位の距離が次の領域との最小距離よりも小さいなら終わり
			if ( mnum >= kn ) {
				if ( merge[kn-1].dist < div[divi+1].dist ) {
					break;
				}
			}
			// そうでなければ次の領域の特徴点を得て繰り返し
			free( tmp0 );
			free( tmp1 );
			tmp1 = NULL;
			tmp0 = merge;	// mergeをtmp0に
			t0num = mnum;
		}
		if ( mnum < kn ) {
			fprintf( stderr, "%d nearest points not found\n", kn );
			return;	// 特徴点数が満たなければエラー
		}
		nears[pi][0] = pi;
		for ( i = 1; i < kn; i++ ) {
			nears[pi][i] = merge[i-1].pindex;
		}
		free( tmp0 );
		if ( tmp1 != NULL )	free( tmp1 );
		if ( merge != tmp0 )	free( merge );
	}
	// メモリ解放
	for ( i = 0; i < tx; i++ ) {
		free( div_pnum[i] );
	}
	free( div_pnum );
	for ( i = 0; i < tx; i++ ) {
		for ( j = 0; j < ty; j++ ) {
			free( div_ps[i][j] );
		}
		free( div_ps[i] );
	}
	free( div_ps );
	free( div );
}
