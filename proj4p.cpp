#include "def_general.h"

#include <stdio.h>
#include "proj4p.h"
#include "gj.h"

void CalcProjParam( strPoint *p1, strPoint *p2, strProjParam *param )
// 対応する4点から射影変換のパラメータを計算する
{
	double a[kProjParamNum][kProjParamNum+1];

	a[0][0] = p1[0].x;
	a[0][1] = 0;
	a[0][2] = -(p1[0].x * p2[0].x);
	a[0][3] = p1[0].y;
	a[0][4] = 0;
	a[0][5] = -(p1[0].y * p2[0].x);
	a[0][6] = 1;
	a[0][7] = 0;
	a[0][8] = p2[0].x;

	a[1][0] = 0;
	a[1][1] = p1[0].x;
	a[1][2] = -(p1[0].x * p2[0].y);
	a[1][3] = 0;
	a[1][4] = p1[0].y;
	a[1][5] = -(p1[0].y * p2[0].y);
	a[1][6] = 0;
	a[1][7] = 1;
	a[1][8] = p2[0].y;

	a[2][0] = p1[1].x;
	a[2][1] = 0;
	a[2][2] = -(p1[1].x * p2[1].x);
	a[2][3] = p1[1].y;
	a[2][4] = 0;
	a[2][5] = -(p1[1].y * p2[1].x);
	a[2][6] = 1;
	a[2][7] = 0;
	a[2][8] = p2[1].x;

	a[3][0] = p1[2].x;
	a[3][1] = 0;
	a[3][2] = -(p1[2].x * p2[2].x);
	a[3][3] = p1[2].y;
	a[3][4] = 0;
	a[3][5] = -(p1[2].y * p2[2].x);
	a[3][6] = 1;
	a[3][7] = 0;
	a[3][8] = p2[2].x;

	a[4][0] = 0;
	a[4][1] = p1[1].x;
	a[4][2] = -(p1[1].x * p2[1].y);
	a[4][3] = 0;
	a[4][4] = p1[1].y;
	a[4][5] = -(p1[1].y * p2[1].y);
	a[4][6] = 0;
	a[4][7] = 1;
	a[4][8] = p2[1].y;

	a[5][0] = 0;
	a[5][1] = p1[2].x;
	a[5][2] = -(p1[2].x * p2[2].y);
	a[5][3] = 0;
	a[5][4] = p1[2].y;
	a[5][5] = -(p1[2].y * p2[2].y);
	a[5][6] = 0;
	a[5][7] = 1;
	a[5][8] = p2[2].y;

	a[6][0] = p1[3].x;
	a[6][1] = 0;
	a[6][2] = -(p1[3].x * p2[3].x);
	a[6][3] = p1[3].y;
	a[6][4] = 0;
	a[6][5] = -(p1[3].y * p2[3].x);
	a[6][6] = 1;
	a[6][7] = 0;
	a[6][8] = p2[3].x;

	a[7][0] = 0;
	a[7][1] = p1[3].x;
	a[7][2] = -(p1[3].x * p2[3].y);
	a[7][3] = 0;
	a[7][4] = p1[3].y;
	a[7][5] = -(p1[3].y * p2[3].y);
	a[7][6] = 0;
	a[7][7] = 1;
	a[7][8] = p2[3].y;

	gj(a);

	param->a1 = a[0][8];
	param->a2 = a[1][8];
	param->a3 = a[2][8];
	param->b1 = a[3][8];
	param->b2 = a[4][8];
	param->b3 = a[5][8];
	param->c1 = a[6][8];
	param->c2 = a[7][8];
}

void ProjTrans( strPoint *src, strPoint *dst, strProjParam *param )
// パラメータで射影変換先の座標を計算する（正対画像の座標src，傾き画像の座標dst）
{
	dst->x = (int)((param->a1 * src->x + param->b1 * src->y + param->c1 ) / ( param->a3 * src->x + param->b3 * src->y + 1.0 )+0.5);
	dst->y = (int)((param->a2 * src->x + param->b2 * src->y + param->c2 ) / ( param->a3 * src->x + param->b3 * src->y + 1.0 )+0.5);
}
