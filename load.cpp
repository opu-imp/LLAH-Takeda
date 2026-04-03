#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <opencv2/core.hpp>

#ifdef WIN32
#ifdef _DEBUG
#pragma comment( lib, "opencv_core4d.lib" )
#else
#pragma comment( lib, "opencv_core4.lib" )
#endif
#endif

#include "def_general.h"
#include "extern.h"
#include "block.h"
#include "hash.h"
#include "init.h"
#include "hcompress.h"

using namespace std;

int LoadPointFile( char *fname, cv::Point ***p_reg_pss, double ***p_reg_areass, cv::Size **p_reg_sizes, int **p_reg_nums, char ***p_dbcors )
// �����_�t�@�C�������[�h����
{
	fprintf( stderr, "Point File ... ");
    int i, j;
	int doc_num;
	cv::Point **reg_pss;
	cv::Size *reg_sizes;
	double **reg_areass;
	int *reg_nums;
	char **dbcors, pname[kMaxPathLen];

	if ( fname == NULL ) {
		sprintf( pname, "%s%s", eDataDir, ePointFileName );
		fname = pname;
	}
	if ( StartReadBlock(fname, kBlockSize) == 0 ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		exit(1);
	}

	if ( ReadBlock((unsigned char *)&doc_num, sizeof(int)) == 0 ) {
		fprintf( stderr, "error: %s registered nothing\n", fname );
		exit(1);
	}
	
	cout << doc_num << endl;

	// �������̊m��
	reg_pss = (cv::Point **)calloc( doc_num, sizeof(cv::Point *) );
	reg_areass = (double **)calloc( doc_num, sizeof(double *) );
	reg_sizes = (cv::Size *)calloc( doc_num, sizeof(cv::Size) );
	reg_nums = (int *)calloc( doc_num, sizeof(int) );
	dbcors = (char **)calloc( doc_num, sizeof(char *) );
	for ( i = 0; i < doc_num; i++ )
		dbcors[i] = (char *)calloc( kMaxPathLen, sizeof(char) );

	// �ۑ�
	for ( i = 0; i < doc_num; i++ ) {
		ReadBlock( (unsigned char *)dbcors[i], kMaxPathLen );
		ReadBlock( (unsigned char *)&reg_sizes[i].width, sizeof(int) );
		ReadBlock( (unsigned char *)&reg_sizes[i].height, sizeof(int) );
		ReadBlock( (unsigned char *)&reg_nums[i], sizeof(int) );
		//cout << dbcors[i] << ":" << reg_nums[i] << ":";
		reg_pss[i] = (cv::Point *)calloc( reg_nums[i], sizeof(cv::Point) );	// �����_�f�[�^�̃������m��
		reg_areass[i] = (double *)calloc( reg_nums[i], sizeof(double) );	// �ʐσf�[�^�̃������m��
		double max=0;
		for ( j = 0; j < reg_nums[i]; j++ ) {
			ReadBlock( (unsigned char *)&reg_pss[i][j].x, sizeof(int) );
			ReadBlock( (unsigned char *)&reg_pss[i][j].y, sizeof(int) );
			ReadBlock( (unsigned char *)&reg_areass[i][j], sizeof(double) );
			//if ( max < reg_areass[i][j] )	max = reg_areass[i][j];
		}

		//cout << max << endl;
	}
	FinishReadBlock();

//	for ( i = 0; i < doc_num; i++ ) {
//		printf("%d, %d\n", reg_sizes[i].width, reg_sizes[i].height);
//	}

	*p_reg_pss = reg_pss;
	*p_reg_areass = reg_areass;
	*p_reg_sizes = reg_sizes;
	*p_reg_nums = reg_nums;
	*p_dbcors = dbcors;
	if ( *p_reg_pss==NULL || *p_reg_areass==NULL || *p_reg_nums==NULL || *p_dbcors==NULL ) {
		fprintf( stderr, "error: failed to pass pointa" );
		exit(1);
	}
	fprintf( stderr, "Loaded\n");
	return doc_num;
}

int LoadPointFile2( char *fname, char ***p_dbcors )
// �����_�t�@�C�������[�h����
{
	//fprintf( stderr, "Point File ... ");
    int i, j;
	int doc_num;
	char **dbcors, pname[kMaxPathLen];

	if ( fname == NULL ) {
		sprintf( pname, "%s%s", eDataDir, "point_comp.dat" );
		fname = pname;
	}
	if ( StartReadBlock(fname, kBlockSize) == 0 ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		exit(1);
	}

	if ( ReadBlock((unsigned char *)&doc_num, sizeof(int)) == 0 ) {
		fprintf( stderr, "error: %s registered nothing\n", fname );
		exit(1);
	}
	// �������̊m��
	dbcors = (char **)calloc( doc_num, sizeof(char *) );
	for ( i = 0; i < doc_num; i++ ) {
		dbcors[i] = (char *)calloc( kMaxPathLen, sizeof(char) );
	}

	// �ۑ�
	for ( i = 0; i < doc_num; i++ ) {
		ReadBlock( (unsigned char *)dbcors[i], kMaxPathLen );
		//cout << dbcors[i] << endl;
	}
	FinishReadBlock();

	*p_dbcors = dbcors;

	//fprintf( stderr, "Loaded\n");
	return doc_num;
}

int LoadThinFile( char *fname, cv::Point ***p_reg_pss, cv::Size **p_reg_sizes, int **p_reg_nums, char ***p_dbcors )
// �����_�t�@�C�������[�h����
{
    int i, j;
	int doc_num;
	cv::Point **reg_pss;
	cv::Size *reg_sizes;
	int *reg_nums;
	char **dbcors, tname[kMaxPathLen];
	
	printf("LoadThinFile!\n");

	if ( fname == NULL ) {
		sprintf( tname, "%s%s", eDataDir, eThinFileName );
		fname = tname;
	}
	if ( StartReadBlock(fname, kBlockSize) == 0 ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		exit(1);
	}

	if ( ReadBlock((unsigned char *)&doc_num, sizeof(int)) == 0 ) {
		fprintf( stderr, "error: %s registered nothing\n", fname );
		exit(1);
	}

	// �������̊m��
	reg_pss = (cv::Point **)calloc( doc_num, sizeof(cv::Point *) );
	reg_sizes = (cv::Size *)calloc( doc_num, sizeof(cv::Size) );
	reg_nums = (int *)calloc( doc_num, sizeof(int) );
	dbcors = (char **)calloc( doc_num, sizeof(char *) );
	for ( i = 0; i < doc_num; i++ )
		dbcors[i] = (char *)calloc( kMaxPathLen, sizeof(char) );

	// �ۑ�
	for ( i = 0; i < doc_num; i++ ) {
		ReadBlock( (unsigned char *)dbcors[i], kMaxPathLen );
		ReadBlock( (unsigned char *)&reg_sizes[i].width, sizeof(int) );
		ReadBlock( (unsigned char *)&reg_sizes[i].height, sizeof(int) );
		ReadBlock( (unsigned char *)&reg_nums[i], sizeof(int) );
		reg_pss[i] = (cv::Point *)calloc( reg_nums[i], sizeof(cv::Point) );	// �����_�f�[�^�̃������m��
		for ( j = 0; j < reg_nums[i]; j++ ) {
			ReadBlock( (unsigned char *)&reg_pss[i][j].x, sizeof(int) );
			ReadBlock( (unsigned char *)&reg_pss[i][j].y, sizeof(int) );
		}
	}
	FinishReadBlock();

	*p_reg_pss = reg_pss;
	*p_reg_sizes = reg_sizes;
	*p_reg_nums = reg_nums;
	*p_dbcors = dbcors;
	if ( *p_reg_pss == NULL || *p_reg_nums == NULL || *p_dbcors == NULL ) {
		fprintf( stderr, "error: failed to pass pointa" );
		exit(1);
	}

	return doc_num;
}


int LoadThinFile2( char *fname, char ***p_dbcors )
// �����_�t�@�C�������[�h����
{
    int i, j;
	int doc_num;
	cv::Point **reg_pss;
	cv::Size *reg_sizes;
	int *reg_nums;
	char **dbcors, tname[kMaxPathLen];
	
	//printf("ThinFile ... ");

	if ( fname == NULL ) {
		sprintf( tname, "%s%s", eDataDir, "thin_comp.dat" );
		fname = tname;
	}
	if ( StartReadBlock(fname, kBlockSize) == 0 ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		exit(1);
	}

	if ( ReadBlock((unsigned char *)&doc_num, sizeof(int)) == 0 ) {
		fprintf( stderr, "error: %s registered nothing\n", fname );
		exit(1);
	}

	// �������̊m��
	dbcors = (char **)calloc( doc_num, sizeof(char *) );
	for ( i = 0; i < doc_num; i++ ) {
		dbcors[i] = (char *)calloc( kMaxPathLen, sizeof(char) );
	}
	// �ۑ�
	for ( i = 0; i < doc_num; i++ ) {
		ReadBlock( (unsigned char *)dbcors[i], kMaxPathLen );
		//cout << dbcors[i] << endl;
	}
	FinishReadBlock();

	*p_dbcors = dbcors;
	//fprintf( stderr, "Loaded\n");

	return doc_num;
}

int LoadQueryPointFile( char *fname, cv::Point **ps0, double **areas0 )
// �N�G�������_�t�@�C���̓ǂݍ���
{
    int i, num;
	double *areas;
	cv::Point *ps;

	if ( StartReadBlock(fname, kBlockSize) == 0 ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		exit(1);
	}
	if ( ReadBlock((unsigned char *)&num, sizeof(int)) == 0 ) {
		fprintf( stderr, "%s was cannot opened\n", fname );
		exit(1);
	}
	//printf("%d\n", doc_num);
	
	// �������̊m��
	ps = (cv::Point *)calloc( num, sizeof(cv::Point) );
	areas = (double *)calloc( num, sizeof(double) );

	for ( i = 0; i < num; i++ ) {
		ReadBlock( (unsigned char *)&ps[i].x, sizeof(int) );
		ReadBlock( (unsigned char *)&ps[i].y, sizeof(int) );
		ReadBlock( (unsigned char *)&areas[i], sizeof(double) );
		//printf("%d, %d, %lf\n", ps[i].x, ps[i].y, areas[i]);
	}

	FinishReadBlock();

	*ps0 = ps;
	*areas0 = areas;
	
	return num;
}

int LoadDocNum( char *fname )
// �o�^�����摜���̓ǂݍ���
{
	int doc_num;
	char tname[kMaxPathLen];

	if ( StartReadBlock(fname, kBlockSize) == 0 ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		exit(1);
	}
	if ( ReadBlock((unsigned char *)&doc_num, sizeof(int)) == 0 ) {
		fprintf( stderr, "error: %s registered nothing\n", fname );
		exit(1);
	}
	FinishReadBlock();

	return doc_num;
}

void LoadDisc( strDisc *disc )
// ���U���t�@�C���̓ǂݍ���
{
	fprintf( stderr, "Disc File ... ");
	int i;
	char fname[kMaxPathLen], line[kDiscLineLen];
	FILE *fp;

	sprintf( fname, "%s%s", eDataDir, eDiscFileName );
	if ( (fp = fopen( fname, "r" )) == NULL ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		exit(1);
	}

	fgets( line, kDiscLineLen, fp );
	sscanf( line, "%lf", &disc->min );
	fgets( line, kDiscLineLen, fp );
	sscanf( line, "%lf", &disc->max );
	fgets( line, kDiscLineLen, fp );
	sscanf( line, "%d", &disc->num );
	fgets( line, kDiscLineLen, fp );
	sscanf( line, "%d", &disc->res );
	disc->dat = (int *)calloc(disc->res, sizeof(int));
	for ( i = 0; i < disc->res; i++ ) {
		fgets( line, kDiscLineLen, fp );
		sscanf( line, "%d", &disc->dat[i] );
	}
	fclose( fp );

	fprintf( stderr, "Loaded\n");
}

void LoadRmRedund( void )
// �����I���t�@�C���̓ǂݍ���
{
	int i, j;
	char line[kMaxLineLen], fname[kMaxPathLen];
	FILE *fp;

	sprintf( fname, "%s%s", eDataDir, "comb.dat" );
	if ( (fp = fopen(fname, "r" )) == NULL ) {
		fprintf( stderr, "%s cannot be opened\n", "comb.dat" );
		exit(1);
	}
	fgets( line, kDiscLineLen, fp );
	for ( i = 0; i < eNumCom2; i++ ) {
		for ( j = 0; j < 4; j++ ) {
			com2[i][j]=line[i*4+j]-48;
		}

	}

	fclose(fp);
}

void LoadHashCompressToStorage( strHash ***p_hash )
// �n�b�V�������[�h����
{
	fprintf( stderr, "Hash File ... ");
//	int start_mem=0, end_mem=0, memsize=0;
	char fname[kMaxPathLen];
	long long int location=0;
	unsigned char n;
	unsigned int index;
	strHash **hash = NULL;
	strHash *storage = NULL;
	int len_list = 0;

	// ���X�g1���̒����iSaveHash�Ɠ����j
	if ( eCompressHash )	len_list = eHComp2DatByte;
	else					len_list = eFByte;

	sprintf( fname, "%s%s", eDataDir, eHashFileName );	// �n�b�V���t�@�C���̃I�[�v��
	if ( StartReadBlock(fname, kBlockSize) == 0 ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		exit(1);
	}

	hash = InitHash();
	if ( hash == NULL ) {
		fprintf( stderr, "error: cannot allocated hash table\n" );
		exit(1);
	}

#ifndef WIN32	
	// �������v���J�n
	//start_mem = CalcMemory();
#endif

	//printf("%lld\n", eHashStorageNum);
	storage = (strHash *)malloc( sizeof(strHash) * eHashStorageNum );
	memset( storage, 0, sizeof(strHash) * eHashStorageNum );

	while ( ReadBlock( (unsigned char *)&index, sizeof(unsigned int) ) != 0 ) {
		hash[index] = &(storage[location]);
		ReadBlock( (unsigned char *)&n, sizeof(unsigned char) );
		storage[location++] = n;
		ReadBlock( (unsigned char *)&(storage[location]), n*len_list);
		location += (n*len_list);
	}

	FinishReadBlock();

	// �|�C���^�n��
	*p_hash = hash;
	if ( *p_hash == NULL ) {
		fprintf( stderr, "error: failed to pass hash table" );
		exit(1);
	}

#ifndef WIN32	
	// �������v���I��
	//end_mem = CalcMemory();
	//memsize = end_mem - start_mem;
	// �g�p�������\��
	//printf("Used Memory Size\t: %15lf KB, %15lf MB, %15lf GB\n", ((double)memsize), ((double)memsize/1024), ((double)memsize/1024/1024) );
#endif
	fprintf( stderr, "Loaded\n");
}


void LoadHashCompress( strHash ***p_hash )
// �n�b�V�������[�h����
{
	unsigned char n;
	unsigned int i, index;
	unsigned char *dat;
	unsigned int pdoc;
	unsigned short ppoint;
	unsigned char pquotient;	// ��
	char fname[kMaxPathLen];
	strHash **hash = NULL;

	dat = (unsigned char *)calloc( eHComp2DatByte, sizeof(unsigned char) );
	
	hash = InitHash();
	if ( hash == NULL ) {
		fprintf( stderr, "error: cannot allocated hash table\n" );
		exit(1);
	}

	sprintf( fname, "%s%s", eDataDir, eHashFileName );	// �n�b�V���t�@�C���̃I�[�v��
	if ( StartReadBlock(fname, kBlockSize) == 0 ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		exit(1);
	}

	while ( ReadBlock((unsigned char *)&index, sizeof(unsigned int)) != 0 ) {
		ReadBlock((unsigned char *)&n, sizeof(unsigned char));
		for ( i = 0; i < n; i++ ) {
			ReadBlock( dat, eHComp2DatByte );
			ReadHComp2Dat( dat, &pdoc, &ppoint, &pquotient );
			AddHashCompress(index, pdoc, ppoint, pquotient, hash);
		}
	}
	FinishReadBlock();
	free( dat );

	// �|�C���^�n��
	*p_hash = hash;
	if ( *p_hash == NULL ) {
		fprintf( stderr, "error: failed to pass hash table" );
		exit(1);
	}

}


void LoadHashCompressForCombine( char *fname, strHash **hash, int doc_num )
// �n�b�V�������[�h����
{
	unsigned char n;
	unsigned int i, index;
	unsigned char *dat;
	unsigned int pdoc;
	unsigned short ppoint;
	unsigned char pquotient;	// ��

	dat = (unsigned char *)calloc( eHComp2DatByte, sizeof(unsigned char) );

	if ( StartReadBlock(fname, kBlockSize) == 0 ) {
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		exit(1);
	}

	while ( ReadBlock((unsigned char *)&index, sizeof(unsigned int)) != 0 ) {
		ReadBlock((unsigned char *)&n, sizeof(unsigned char));
		for ( i = 0; i < n; i++ ) {
			ReadBlock( dat, eHComp2DatByte );
			ReadHComp2Dat( dat, &pdoc, &ppoint, &pquotient );
			AddHashCompress(index, pdoc+doc_num, ppoint, pquotient, hash);
		}
	}
	FinishReadBlock();
	free( dat );
}

void LoadConfig( void )
// �ݒ�t�@�C���̓ǂݍ���
{
	fprintf( stderr, "Config File ... ");
	char fname[kMaxPathLen], line[kMaxLineLen], ch, *fgets_ret=NULL;
	FILE *fp=NULL;

	sprintf( fname, "%s%s", eDataDir, eConfigFileName );	// config.dat�̐�΃p�X���쐬����
	if ( (fp = fopen(fname, "r")) == NULL ) {				// �J��
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		exit(1);
	}

	fgets_ret = fgets( line, kMaxLineLen, fp );					// 1�s�ǂݍ���
	if ( fgets_ret == NULL || !strcmp( line, kConfigVerStr ) ) {	// �ǂݍ��ݎ��s�܂��̓o�[�W�����Ⴂ
		fprintf( stderr, "error: %s version error\n", eConfigFileName );
		exit(1);
	}

	// �e�s����͂��Đݒ�
	/*	��:	n 7
			m 6
			d 15	*/
	while ( fgets( line, kMaxLineLen, fp ) != NULL ) {
		switch ( line[0] ) {
			case 'n':	// n
				sscanf( line, "%c %d", &ch, &eGroup1Num );
				break;
			case 'm':	// m
				sscanf( line, "%c %d", &ch, &eGroup2Num );
				break;
			case 'd':	// ���U�����x��
				sscanf( line, "%c %d", &ch, &eDiscNum );
				break;
			case 'p':	// ���萔
				sscanf( line, "%c %lf", &ch, &eProp );
				break;
			case 'H':	// �n�b�V���e�[�u���̃T�C�Y
				sscanf( line, "%c %llu %lld", &ch, &eHashSize, &eHashStorageNum );
				break;
			case 'a':	// �ʐϔ�����ʎg�p
				eUseArea = 1;
				break;
			case 'r':	// �����ʑI��L�i�璷�������j
				eRmRedund = 1;
				break;
			case 't':	// �����_�T���v�����O�L
				eThinPs = 1;
				break;
			case 'z':	// ���X�g���k
				eCompressHash = 1;
				sscanf( line, "%c %d %d %d %d", &ch, &eDocBit, &ePointBit, &eQuotBit, &eHComp2DatByte );
				break;
			default:
				break;
		}
	}
	fclose( fp );

	// �g�����v�Z
	eNumCom1 = CalcnCr( eGroup1Num, eGroup2Num );	// nCm
	eNumCom2 = CalcnCr( eGroup2Num, eGroup3Num );	// mCf

	eFByte = sizeof(int) + sizeof(short) + sizeof(unsigned char);	// ���k�����̃��X�g�T�C�Y�i�ꉞ�j
	fprintf( stderr, "Loaded\n");
}

void LoadConfigForCombine( char *fname )
// �ݒ�t�@�C���̓ǂݍ���
{
	fprintf( stderr, "Config File ... ");
	char line[kMaxLineLen], ch, *fgets_ret=NULL;
	FILE *fp=NULL;

	if ( (fp = fopen(fname, "r")) == NULL ) {				// �J��
		fprintf( stderr, "error: %s cannot be opened\n", fname );
		exit(1);
	}

	fgets_ret = fgets( line, kMaxLineLen, fp );					// 1�s�ǂݍ���
	if ( fgets_ret == NULL || !strcmp( line, kConfigVerStr ) ) {	// �ǂݍ��ݎ��s�܂��̓o�[�W�����Ⴂ
		fprintf( stderr, "error: %s version error\n", eConfigFileName );
		exit(1);
	}

	// �e�s����͂��Đݒ�
	/*	��:	n 7
			m 6
			d 15	*/
	while ( fgets( line, kMaxLineLen, fp ) != NULL ) {
		switch ( line[0] ) {
			case 'n':	// n
				sscanf( line, "%c %d", &ch, &eGroup1Num );
				break;
			case 'm':	// m
				sscanf( line, "%c %d", &ch, &eGroup2Num );
				break;
			case 'd':	// ���U�����x��
				sscanf( line, "%c %d", &ch, &eDiscNum );
				break;
			case 'p':	// ���萔
				sscanf( line, "%c %lf", &ch, &eProp );
				break;
			case 'H':	// �n�b�V���e�[�u���̃T�C�Y
				sscanf( line, "%c %llu %lld", &ch, &eHashSize, &eHashStorageNum );
				break;
			case 'a':	// �ʐϔ�����ʎg�p
				eUseArea = 1;
				break;
			case 'r':	// �����ʑI��L�i�璷�������j
				eRmRedund = 1;
				break;
			case 't':	// �����_�T���v�����O�L
				eThinPs = 1;
				break;
			case 'z':	// ���X�g���k
				eCompressHash = 1;
				sscanf( line, "%c %d %d %d %d", &ch, &eDocBit, &ePointBit, &eQuotBit, &eHComp2DatByte );
				break;
			default:
				break;
		}
	}
	fclose( fp );

	// �g�����v�Z
	eNumCom1 = CalcnCr( eGroup1Num, eGroup2Num );	// nCm
	eNumCom2 = CalcnCr( eGroup2Num, eGroup3Num );	// mCf

	eFByte = sizeof(int) + sizeof(short) + sizeof(unsigned char);	// ���k�����̃��X�g�T�C�Y�i�ꉞ�j
	fprintf( stderr, "Loaded\n");
}