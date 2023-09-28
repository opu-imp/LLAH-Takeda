#ifndef _LLAHDOC_PROJ4P_H_
#define _LLAHDOC_PROJ4P_H_

typedef int keytype;

typedef struct _strPoint {
	int x;
	int y;
} strPoint;

#define	kProjParamNum	8
#define	kLinearVarNum	kProjParamNum
#define	kTryParamRand	(100)	/* �����_���Ɏˉe�ϊ��̃p�����[�^�̐�����s���� */

void CalcProjParam( strPoint *p1, strPoint *p2, strProjParam *param );
void ProjTrans( strPoint *src, strPoint *dst, strProjParam *param );
void MeanParam( double data[kTryParamRand][8], double *mean, int n, double err0, int threshold);
void quicksort(double *d, keytype *a, int first, int last);
void simplesort(double *d, keytype *a, int first, int last);
void GetAppropriateParam( strProjParam *paramarr, strProjParam *param );
double GetPPVarSub( strProjParam *paramarr );
double GetPPVarSubSub( double data[kTryParamRand][8], int n, double err0, int threshold );

#endif
