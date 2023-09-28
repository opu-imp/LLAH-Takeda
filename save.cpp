#include "def_general.h"

#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp> 

#ifdef WIN32
#ifdef _DEBUG
#pragma comment( lib, "opencv_imgproc231d.lib" )
#pragma comment( lib, "opencv_core231d.lib" )
#pragma comment( lib, "opencv_highgui231d.lib" )
#else
#pragma comment( lib, "opencv_imgproc231.lib" )
#pragma comment( lib, "opencv_core231.lib" )
#pragma comment( lib, "opencv_highgui231.lib" )
#endif
#endif

#include "extern.h"
#include "block.h"
#include "fpath.h"

extern strHash cut;

void SavePointFile( int doc_num, char **dbcors, int *nums, CvPoint **pss, double **areass, CvSize *sizes )
// 特徴点ファイルを保存
{
	int i, j;
	char fname[kMaxPathLen];

	sprintf( fname, "%s%s", eDataDir, ePointFileName );
	StartWriteBlock( fname, kBlockSize );	// 公開に向けて改変

	WriteBlock( (unsigned char *)&doc_num, sizeof(int) );
	for ( i = 0; i < doc_num; i++ ) {
		WriteBlock( (unsigned char *)dbcors[i], kMaxPathLen );
		WriteBlock( (unsigned char *)&sizes[i].width, sizeof(int) );
		WriteBlock( (unsigned char *)&sizes[i].height, sizeof(int) );
		WriteBlock( (unsigned char *)&nums[i], sizeof(int) );
		for ( j = 0; j < nums[i]; j++ ) {
			WriteBlock( (unsigned char *)&pss[i][j].x, sizeof(int) );
			WriteBlock( (unsigned char *)&pss[i][j].y, sizeof(int) );
			WriteBlock( (unsigned char *)&areass[i][j], sizeof(double) );
		}
	}
	FinishWriteBlock();
}

void SavePointFile2( int doc_num, char **dbcors )
// 特徴点ファイルを保存
{
	int i, j;
	char fname[kMaxPathLen];

	sprintf( fname, "%s%s", eDataDir, "point_comp.dat" );
	StartWriteBlock( fname, kBlockSize );	// 公開に向けて改変

	WriteBlock( (unsigned char *)&doc_num, sizeof(int) );
	for ( i = 0; i < doc_num; i++ ) {
		WriteBlock( (unsigned char *)dbcors[i], kMaxPathLen );
	}
	FinishWriteBlock();
}

void SavePointFileText( int doc_num, char **dbcors, int *nums, CvPoint **pss, double **areass, CvSize *sizes )
// 特徴点ファイルを保存
{
	int i, j;
	char fname[kMaxPathLen];
	FILE *fp;

	sprintf( fname, "%s%s", eDataDir, ePointFileName );
	if ( (fp = fopen( fname, "w" )) == NULL ) {
		fprintf( stderr, "error : %s cannot be opened\n", fname );
	}
	// ファイル数の出力
	fprintf( fp, "%d\n", eDbDocs );

	for ( i = 0; i < doc_num; i++ ) {
		fprintf( fp, "%s\n", dbcors[i] );	// 元画像ファイル名の出力
		fprintf( fp, "%d,%d\n", sizes[i].width, sizes[i].height );
		fprintf( fp, "%d\n", nums[i] );	// 特徴点数を出力
		for ( j = 0; j < nums[i]; j++ ) {
			fprintf( fp, "%d,%d,%lf\n", pss[i][j].x, pss[i][j].y, areass[i][j] );	// 特徴点の座標・連結成分の面積を出力
		}
	}
	fclose( fp );
}

void SaveThinFile( int doc_num, int **thinids, CvPoint **pss, int *thin_nums, CvSize *sizes, char **dbcors )
// サンプリング後の特徴点数を保存
{
	int i, j;
	char fname[kMaxPathLen];

	sprintf( fname, "%s%s", eDataDir, eThinFileName );
	StartWriteBlock( fname, kBlockSize );	// 公開に向けて改変

	WriteBlock( (unsigned char *)&doc_num, sizeof(int) );
	for ( i = 0; i < doc_num; i++ ) {
		WriteBlock( (unsigned char *)dbcors[i], kMaxPathLen );
		WriteBlock( (unsigned char *)&sizes[i].width, sizeof(int) );
		WriteBlock( (unsigned char *)&sizes[i].height, sizeof(int) );
		WriteBlock( (unsigned char *)&thin_nums[i], sizeof(int) );
		for ( j = 0; j < thin_nums[i]; j++ ) {
			WriteBlock( (unsigned char *)&pss[i][thinids[i][j]].x, sizeof(int) );
			WriteBlock( (unsigned char *)&pss[i][thinids[i][j]].y, sizeof(int) );
		}
	}
	FinishWriteBlock();
}


void SaveThinFile2( int doc_num, char **dbcors )
// サンプリング後の特徴点数を保存
{
	int i, j;
	char fname[kMaxPathLen];

	sprintf( fname, "%s%s", eDataDir, "thin_comp.dat" );
	StartWriteBlock( fname, kBlockSize );	// 公開に向けて改変

	WriteBlock( (unsigned char *)&doc_num, sizeof(int) );
	for ( i = 0; i < doc_num; i++ ) {
		WriteBlock( (unsigned char *)dbcors[i], kMaxPathLen );
	}
	FinishWriteBlock();
}

void SaveThinFileForCombine( int doc_num, CvPoint **pss, CvSize *sizes, int *thin_nums, char **dbcors )
// サンプリング後の特徴点数を保存
{
	int i, j;
	char pthin_fname[kMaxPathLen];

	sprintf( pthin_fname, "%s%s", eDataDir, eThinFileName );
	StartWriteBlock( pthin_fname, kBlockSize );	// 公開に向けて改変

	WriteBlock( (unsigned char *)&doc_num, sizeof(int) );
	for ( i = 0; i < doc_num; i++ ) {
		WriteBlock( (unsigned char *)dbcors[i], kMaxPathLen );
		WriteBlock( (unsigned char *)&sizes[i].width, sizeof(int) );
		WriteBlock( (unsigned char *)&sizes[i].height, sizeof(int) );
		WriteBlock( (unsigned char *)&thin_nums[i], sizeof(int) );
		for ( j = 0; j < thin_nums[i]; j++ ) {
			WriteBlock( (unsigned char *)&pss[i][j].x, sizeof(int) );
			WriteBlock( (unsigned char *)&pss[i][j].y, sizeof(int) );
		}
	}
	FinishWriteBlock();
}

void SaveDisc( strDisc *disc )
// 離散化ファイルを保存
{
	char fname[kMaxPathLen];
	int i;
	FILE *fp;
	
	sprintf( fname, "%s%s", eDataDir, eDiscFileName );
	if ( (fp = fopen(fname, "w")) == NULL )	{
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		exit(1);
	}
	fprintf(fp, "%f\n", disc->min);
	fprintf(fp, "%f\n", disc->max);
	fprintf(fp, "%d\n", disc->num);
	fprintf(fp, "%d\n", disc->res);
	for ( i = 0; i < disc->res; i++ )	fprintf(fp, "%d\n", disc->dat[i]);

	fclose(fp);

	// バイナリ版
	//sprintf( fname, "%s%s", eDataDir, eDiscFileName );
	//StartWriteBlock( fname, kBlockSize );
	//WriteBlock( (unsigned char *)&disc->min, sizeof(double) );
	//WriteBlock( (unsigned char *)&disc->max, sizeof(double) );
	//WriteBlock( (unsigned char *)&disc->num, sizeof(int) );
	//WriteBlock( (unsigned char *)&disc->res, sizeof(int) );
	//for ( i = 0; i < disc->res; i++ )	WriteBlock( (unsigned char *)&disc->dat[i], sizeof(int) );
	//FinishWriteBlock();
}

void SaveHash( strHash **hash )
// ハッシュをセーブする
{
	unsigned int i;
	char fname[kMaxPathLen];
	int len_list=0;
	eHashStorageNum = 0;

	// リスト1つ分の長さ
	if ( eCompressHash )	len_list = eHComp2DatByte;
	else					len_list = eFByte;

	sprintf( fname, "%s%s", eDataDir, eHashFileName );
	StartWriteBlock( fname, kBlockSize );	// 公開に向けて改変
	for ( i = 0; i < eHashSize; i++ ) {
		if( hash[i] != NULL && hash[i] != &cut ) {
			printf("%d\n", i);
			WriteBlock( ( unsigned char * )&i, sizeof(unsigned int) );
			WriteBlock( ( unsigned char * )hash[i], 1 + ( *( hash[i] ) ) * len_list );
			eHashStorageNum += 1 + ( *( hash[i] ) ) * len_list;
		}
	}
	FinishWriteBlock();
}

void SaveHashText( strHash **hash )
// ハッシュをセーブする
{
	unsigned int i;
	char fname[kMaxPathLen];
	int len_list=0;
	eHashStorageNum = 0;
	FILE *fp;
	int num;
	int doc;
	short point;
	unsigned char quotient;

	// リスト1つ分の長さ
	if ( eCompressHash )	len_list = eHComp2DatByte;
	else					len_list = eFByte;

	sprintf( fname, "%s%s", eDataDir, eHashFileName );
	if ( (fp = fopen( fname, "w" )) == NULL ) {
		fprintf( stderr, "error : %s cannot be opened\n", fname );
	}

	for ( i = 0; i < eHashSize; i++ ) {
		if( hash[i] != NULL && hash[i] != &cut ) {
			fprintf(fp, "%d\n", i);
			memcpy( &num, hash[i], sizeof(char));
			fprintf(fp, "%d\n", num);
			for ( int j = 0; j < num; j++ ) {
				memcpy( &doc, hash[i]+1+7*j, sizeof(int) );
				memcpy( &point, hash[i]+1+4+7*j, sizeof(short) );
				memcpy( &quotient, hash[i]+1+6+7*j, sizeof(unsigned char) );
				fprintf(fp, "%d,%d,%u\n", doc, point, quotient);
			}
		}
	}
	fclose(fp);
}

void SaveHashCompress( strHash **hash )
// ハッシュをセーブする
{
	unsigned int i;
	char fname[kMaxPathLen];

	eHashStorageNum = 0;

	sprintf( fname, "%s%s", eDataDir, eHashFileName );
	StartWriteBlock( fname, kBlockSize );	// 公開に向けて改変
	for ( i = 0; i < eHashSize; i++ ) {
		if( hash[i] == NULL ) {

		} else if ( hash[i] != &cut ) {
			WriteBlock( ( unsigned char * )&i, sizeof(unsigned int) );
			WriteBlock( ( unsigned char * )hash[i], 1 + ( *( hash[i] ) ) * eHComp2DatByte );
			eHashStorageNum += 1 + ( *( hash[i] ) ) * eHComp2DatByte;
		}
	}
	FinishWriteBlock();
}

void SaveConfig( void )
// 設定ファイルを保存
{
	char fname[kMaxPathLen];
	FILE *fp;

	// config.datのパスを作成
	sprintf( fname, "%s%s", eDataDir, eConfigFileName );
	if ( ( fp = fopen( fname, "w" ) ) == NULL ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		exit(1);
	}
	
	// config.datのバージョン
	fprintf( fp, "%s\n", kConfigVerStr );
	// n
	fprintf( fp, "%c %d\n", 'n', eGroup1Num );
	// m
	fprintf( fp, "%c %d\n", 'm', eGroup2Num );
	// d
	fprintf( fp, "%c %d\n", 'd', eDiscNum );
	// prop
	fprintf( fp, "%c %lf\n", 'p', eProp );
	// ハッシュサイズ
	fprintf( fp, "%c %llu %lld\n", 'H', eHashSize, eHashStorageNum );
	// ハッシュ圧縮モードならビット数など
	if ( eCompressHash )	fprintf( fp, "%c %d %d %d %d\n", 'z', eDocBit, ePointBit, eQuotBit, eHComp2DatByte );
	// 特徴点間引き
	if ( eThinPs )	fprintf( fp, "%c\n", 't' );
	// 中心点利用
	if ( eRmRedund )	fprintf( fp, "%c\n", 'r' );
	// 面積比特徴量
	if ( eUseArea )	fprintf( fp, "%c\n", 'a' );

	fclose( fp );
}

int SaveQueryPointFile( char *fname, int num, CvPoint *ps, double *areas )
// サンプリング後の特徴点数を保存
{
	int i, ret;
	char base[kMaxPathLen];
	char dat_fname[kMaxPathLen];

	GetBasename( fname, kMaxPathLen, base );
	//puts(base);

	sprintf( dat_fname, "%s%s.%s", eDataDir, base, "dat" );
	//puts(dat_fname);

	StartWriteBlock( dat_fname, kBlockSize );	// 公開に向けて改変

	WriteBlock( (unsigned char *)&num, sizeof(int) );
	for ( i = 0; i < num; i++ ) {
		WriteBlock( (unsigned char *)&ps[i].x, sizeof(int) );
		WriteBlock( (unsigned char *)&ps[i].y, sizeof(int) );
		WriteBlock( (unsigned char *)&areas[i], sizeof(double) );
	}

	FinishWriteBlock();

	return 1;
}