#include "def_general.h"

#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp> 

#ifdef WIN32
#ifdef _DEBUG
#pragma comment( lib, "opencv_imgproc4d.lib" )
#pragma comment( lib, "opencv_core4d.lib" )
#pragma comment( lib, "opencv_highgui4d.lib" )
#else
#pragma comment( lib, "opencv_imgproc4.lib" )
#pragma comment( lib, "opencv_core4.lib" )
#pragma comment( lib, "opencv_highgui4.lib" )
#endif
#endif

#include "extern.h"
#include "block.h"
#include "fpath.h"

extern strHash cut;

void SavePointFile( int doc_num, char **dbcors, int *nums, cv::Point **pss, double **areass, cv::Size *sizes )
// �����_�t�@�C����ۑ�
{
	int i, j;
	char fname[kMaxPathLen];

	sprintf( fname, "%s%s", eDataDir, ePointFileName );
	StartWriteBlock( fname, kBlockSize );	// ���J�Ɍ����ĉ���

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
// �����_�t�@�C����ۑ�
{
	int i, j;
	char fname[kMaxPathLen];

	sprintf( fname, "%s%s", eDataDir, "point_comp.dat" );
	StartWriteBlock( fname, kBlockSize );	// ���J�Ɍ����ĉ���

	WriteBlock( (unsigned char *)&doc_num, sizeof(int) );
	for ( i = 0; i < doc_num; i++ ) {
		WriteBlock( (unsigned char *)dbcors[i], kMaxPathLen );
	}
	FinishWriteBlock();
}

void SavePointFileText( int doc_num, char **dbcors, int *nums, cv::Point **pss, double **areass, cv::Size *sizes )
// �����_�t�@�C����ۑ�
{
	int i, j;
	char fname[kMaxPathLen];
	FILE *fp;

	sprintf( fname, "%s%s", eDataDir, ePointFileName );
	if ( (fp = fopen( fname, "w" )) == NULL ) {
		fprintf( stderr, "error : %s cannot be opened\n", fname );
	}
	// �t�@�C�����̏o��
	fprintf( fp, "%d\n", eDbDocs );

	for ( i = 0; i < doc_num; i++ ) {
		fprintf( fp, "%s\n", dbcors[i] );	// ���摜�t�@�C�����̏o��
		fprintf( fp, "%d,%d\n", sizes[i].width, sizes[i].height );
		fprintf( fp, "%d\n", nums[i] );	// �����_�����o��
		for ( j = 0; j < nums[i]; j++ ) {
			fprintf( fp, "%d,%d,%lf\n", pss[i][j].x, pss[i][j].y, areass[i][j] );	// �����_�̍��W�E�A�������̖ʐς��o��
		}
	}
	fclose( fp );
}

void SaveThinFile( int doc_num, int **thinids, cv::Point **pss, int *thin_nums, cv::Size *sizes, char **dbcors )
// �T���v�����O��̓����_����ۑ�
{
	int i, j;
	char fname[kMaxPathLen];

	sprintf( fname, "%s%s", eDataDir, eThinFileName );
	StartWriteBlock( fname, kBlockSize );	// ���J�Ɍ����ĉ���

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
// �T���v�����O��̓����_����ۑ�
{
	int i, j;
	char fname[kMaxPathLen];

	sprintf( fname, "%s%s", eDataDir, "thin_comp.dat" );
	StartWriteBlock( fname, kBlockSize );	// ���J�Ɍ����ĉ���

	WriteBlock( (unsigned char *)&doc_num, sizeof(int) );
	for ( i = 0; i < doc_num; i++ ) {
		WriteBlock( (unsigned char *)dbcors[i], kMaxPathLen );
	}
	FinishWriteBlock();
}

void SaveThinFileForCombine( int doc_num, cv::Point **pss, cv::Size *sizes, int *thin_nums, char **dbcors )
// �T���v�����O��̓����_����ۑ�
{
	int i, j;
	char pthin_fname[kMaxPathLen];

	sprintf( pthin_fname, "%s%s", eDataDir, eThinFileName );
	StartWriteBlock( pthin_fname, kBlockSize );	// ���J�Ɍ����ĉ���

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
// ���U���t�@�C����ۑ�
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

	// �o�C�i����
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
// �n�b�V�����Z�[�u����
{
	unsigned int i;
	char fname[kMaxPathLen];
	int len_list=0;
	eHashStorageNum = 0;

	// ���X�g1���̒���
	if ( eCompressHash )	len_list = eHComp2DatByte;
	else					len_list = eFByte;

	sprintf( fname, "%s%s", eDataDir, eHashFileName );
	StartWriteBlock( fname, kBlockSize );	// ���J�Ɍ����ĉ���
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
// �n�b�V�����Z�[�u����
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

	// ���X�g1���̒���
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
// �n�b�V�����Z�[�u����
{
	unsigned int i;
	char fname[kMaxPathLen];

	eHashStorageNum = 0;

	sprintf( fname, "%s%s", eDataDir, eHashFileName );
	StartWriteBlock( fname, kBlockSize );	// ���J�Ɍ����ĉ���
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
// �ݒ�t�@�C����ۑ�
{
	char fname[kMaxPathLen];
	FILE *fp;

	// config.dat�̃p�X���쐬
	sprintf( fname, "%s%s", eDataDir, eConfigFileName );
	if ( ( fp = fopen( fname, "w" ) ) == NULL ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		exit(1);
	}
	
	// config.dat�̃o�[�W����
	fprintf( fp, "%s\n", kConfigVerStr );
	// n
	fprintf( fp, "%c %d\n", 'n', eGroup1Num );
	// m
	fprintf( fp, "%c %d\n", 'm', eGroup2Num );
	// d
	fprintf( fp, "%c %d\n", 'd', eDiscNum );
	// prop
	fprintf( fp, "%c %lf\n", 'p', eProp );
	// �n�b�V���T�C�Y
	fprintf( fp, "%c %llu %lld\n", 'H', eHashSize, eHashStorageNum );
	// �n�b�V�����k���[�h�Ȃ�r�b�g���Ȃ�
	if ( eCompressHash )	fprintf( fp, "%c %d %d %d %d\n", 'z', eDocBit, ePointBit, eQuotBit, eHComp2DatByte );
	// �����_�Ԉ���
	if ( eThinPs )	fprintf( fp, "%c\n", 't' );
	// ���S�_���p
	if ( eRmRedund )	fprintf( fp, "%c\n", 'r' );
	// �ʐϔ������
	if ( eUseArea )	fprintf( fp, "%c\n", 'a' );

	fclose( fp );
}

int SaveQueryPointFile( char *fname, int num, cv::Point *ps, double *areas )
// �T���v�����O��̓����_����ۑ�
{
	int i, ret;
	char base[kMaxPathLen];
	char dat_fname[kMaxPathLen];

	GetBasename( fname, kMaxPathLen, base );
	//puts(base);

	sprintf( dat_fname, "%s%s.%s", eDataDir, base, "dat" );
	//puts(dat_fname);

	StartWriteBlock( dat_fname, kBlockSize );	// ���J�Ɍ����ĉ���

	WriteBlock( (unsigned char *)&num, sizeof(int) );
	for ( i = 0; i < num; i++ ) {
		WriteBlock( (unsigned char *)&ps[i].x, sizeof(int) );
		WriteBlock( (unsigned char *)&ps[i].y, sizeof(int) );
		WriteBlock( (unsigned char *)&areas[i], sizeof(double) );
	}

	FinishWriteBlock();

	return 1;
}