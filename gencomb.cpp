/*		comb.c		組合せ(nＣk)の生成	generation of combination		*/
#include "def_general.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "extern.h"

#define		set			unsigned int

set K, N, x, overx;

void gencomb( int n, int k, void (*setnum)(int, int, int) );
int subcomb( int p[] );	/* gencomb1() のスレーブルーチン */
set nextset( void );		/* gencomb1() のスレーブルーチン */

void SetCom1( int i, int j, int n )
// com1のi行j列にnを入れる．getcomb用
{
	com1[i][j] = n;
}

void SetCom2( int i, int j, int n )
// com2のi行j列にnを入れる．getcomb用
{
	com2[i][j] = n;
}

void InitCom( int ***com0, int num1, int num2 )
// com1/com2を初期化する
{
	int i, **com;
	com = (int **)calloc( num1, sizeof(int *) );
	for ( i = 0; i < num1; i++ ) {
		com[i] = (int *)calloc( num2, sizeof(int) );
	}
	*com0 = com;
}

void ReleaseCom( int ***com0, int num2 )
// com1/com2を解放する
{
	int i;
	for ( i = 0; i < num2; i++ )
		free( (*com0)[i] );
	free( *com0 );

}

void GenerateCombination( int g1, int g2, int g3, void (*setcom1)(int, int, int), void (*setcom2)(int, int, int))
{
	InitCom( &com1, eNumCom1, eGroup2Num );
	InitCom( &com2, eNumCom2, eGroup3Num );
	gencomb(g1, g2, setcom1);
	gencomb(g2, g3, setcom2);
}

void ReleaseCombination()
{
	ReleaseCom( &com1, eGroup2Num );
	ReleaseCom( &com2, eGroup3Num );
}

void gencomb(int n, int k, void (*setnum)(int, int, int))
{
	int i, j, *p, *q, count;
	set s;

	p = (int *)malloc(n*sizeof(int));
	count = 0;
	N = n;
	K = k;
	x = (1 << K) - 1L;
	overx = ~((1 << N) - 1L);
	for(j = 1, q = p, s = x; (set)j <= N; j++)
	{
		if(s & 1)	*q++ = j;
		s >>= 1;
	}
	for(i = 0, q = p; i < k; i++)	setnum(count, i, (*q++)-1);
	count++;
	while(subcomb(p))  {
		for(i = 0, q = p; i < k; i++) 	setnum(count, i, (*q++)-1);
		count++;
	}
	free(p);
}

int subcomb(int p[])
{
	int j, *q;
	set s;

	x = nextset();
	if(x & overx)	return 0;
	for(j = 1, q = p, s = x; (set)j <= N; j++)
	{
		if(s & 1)	*q++ = j;
		s >>= 1;
	}
	return 1;
}

set nextset(void)
{
	set smallest, ripple, new_smallest, ones;

	smallest = x & -x;
	ripple = x + smallest;
	new_smallest = ripple & -ripple;
	ones = ((new_smallest / smallest) >> 1) - 1;
	return ripple | ones;
}
