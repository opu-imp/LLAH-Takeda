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
#include "fpath.h"
#include "save.h"
#include "load.h"
#include "procimg.h"
#include "disc.h"
#include "hist.h"
#include "gencomb.h"
#include "nears.h"
#include "affine.h"
#include "hash.h"
#include "outputimg.h"
#include "init.h"

#define CLRa

void CreatePointFile( CvPoint ***p_reg_pss, double ***p_reg_areass, CvSize **p_reg_sizes, int **p_reg_nums, char ***p_dbcors )
// �����_���o����
{
	char search_path[kMaxPathLen];
	char **files=NULL, **dbcors=NULL;
	int i;
	int *reg_nums=NULL;
	double **reg_areass=NULL;
	CvPoint **reg_pss=NULL;
	CvSize *reg_sizes=NULL;
	IplImage *img;
#ifdef CLR
	IplImage *fp_img, *con_img, *orig_img;
#endif

	sprintf( search_path, "%s*.%s", eSrcDir, eSrcSuffix );	// �����p�X�̍쐬
	if ( !eExperimentMode)	fprintf( stdout, "search_path : %s\n", search_path );

	eDbDocs = FindPath( search_path, &files );
	if ( eDbDocs <= 0 )	{
		fprintf( stderr, "error: images not exit\n" );
		exit(1);
	}
	if ( eDbDocs > kMaxDocNum ) {
		fprintf( stderr, "error : images(%d) exceeds max(%d)\n", eDbDocs, kMaxDocNum );
		exit(1);
	}

	// �������̊m��
	reg_pss = (CvPoint **)calloc(eDbDocs, sizeof(CvPoint *));
	reg_areass = (double **)calloc(eDbDocs, sizeof(double *));
	reg_sizes = (CvSize *)calloc(eDbDocs, sizeof(CvSize));
	reg_nums = (int *)calloc(eDbDocs, sizeof(int));
	dbcors = (char **)calloc(eDbDocs, sizeof(char *));
	for ( i = 0; i < eDbDocs; i++ )
		dbcors[i] = (char *)calloc(kMaxPathLen, sizeof(char));

	// �����_���o����
	for ( i = 0; i < eDbDocs; i++ ) {
		//if ( !eExperimentMode )	fprintf( stderr, "%d/%d %s\n", i+1, eDbDocs, files[i] );
		
		strcpy( dbcors[i], files[i] );		// ���摜�t�@�C�����̕ۑ�
		
		// �����摜�̐���
		img = GetConnectedImage( files[i] );
		if ( img == NULL )	continue;
		// �����_���o�E�A�������̖ʐόv�Z
		reg_nums[i] = MakeFeaturePoint( img, &(reg_pss[i]), &(reg_areass[i]), &(reg_sizes[i]) );
		//if ( i == 0 ) {
		//	for ( int j = 0; j < reg_nums[i]; j++ ) {
		//		printf("%3d: %3d, %3d\n", j, reg_pss[0][j].x, reg_pss[0][j].y);
		//	}
		//}
#ifdef CLR
		con_img = cvCloneImage( img );
		orig_img = cvLoadImage( files[i], 0 );
		fp_img = MakeFeaturePointImage( orig_img, con_img, reg_pss[i], reg_nums[i] );
		OutPutImage( fp_img );
		cvReleaseImage( &orig_img );
		cvReleaseImage( &con_img );
		cvReleaseImage( &fp_img );
#endif
		cvReleaseImage( &img );
	}
	//printf("%d, %d\n", reg_pss[0][0].x, reg_pss[0][0].y);
	// �����_�t�@�C���̕ۑ��i�o�C�i���j
	SavePointFile( eDbDocs, dbcors, reg_nums, reg_pss, reg_areass, reg_sizes );
	//SavePointFileText( eDbDocs, dbcors, reg_nums, reg_pss, reg_areass, reg_sizes );

	*p_reg_pss = reg_pss;
	*p_reg_areass = reg_areass;
	*p_reg_sizes = reg_sizes;
	*p_reg_nums = reg_nums;
	*p_dbcors = dbcors;
	
	// ���
	for ( i = 0; i < eDbDocs; i++ ) {
		free( files[i] );
	}
	free( files );
}

void MakeDiscFile( int doc_num, CvPoint **pss, int *nums, strDisc *disc )
// ���U���t�@�C���쐬
{
	int i;
	strHist hist;

	// �q�X�g�O�����̏�����
	InitHist( &hist, eDiscRes, eDiscMin, eDiscMax );
	// �g�����̍쐬
	GenerateCombination( eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
	// �����I��L
	if ( eRmRedund ) {
		eNumCom2 = 17;
		LoadRmRedund();
	}

	for ( i = 0; i < doc_num; i++ ) {
		if ( !eExperimentMode )	fprintf( stdout, "%d/%d\n", i+1, doc_num );
		// �ߖT�\���̉��
		MakeNearsFromCentres( pss[i], nums[i] );
		// �q�X�g�O�����̍쐬
		CalcAffineAndAddHist( pss[i], nums[i], &hist );
		// ���Ƃ��܂�
		ClearNears( nums[i] );
	}

	// �g�����̉��
	ReleaseCombination();
	if ( eRmRedund )	eNumCom2 = CalcnCr( eGroup2Num, eGroup3Num );

	// �A�t�B���s�ϗʃq�X�g�O�������痣�U����臒l�����߂�
	if ( eTrueHist )	Hist2Disc2( &hist, disc, eDiscNum );
	else				Hist2Disc( &hist, disc, eDiscNum );
	// ���U���t�@�C���̕ۑ�
	SaveDisc( disc );
}

void ConstructHash( CvPoint **pss, double **areass, CvSize *sizes, int doc_num, int *nums, strDisc *disc, strHash ***ptr_hash, char **dbcors )
// �n�b�V�����\�z
{
	int i, j;
	int **thinids = NULL, *thin_nums = NULL, *ids = NULL;
	strHash **hash = NULL;
	int *collision=NULL;

	if ( eThinPs ) {
		thinids = (int **)calloc( eDbDocs, sizeof(int *) );
		for ( i = 0; i < eDbDocs; i++ )	thinids[i] = (int *)calloc( 500, sizeof(int) );
		thin_nums = (int *)calloc( eDbDocs, sizeof(int) );
	} else {
		ids = (int *)calloc( kMaxPointNum, sizeof(int) );
	}

	// �n�b�V���̏�����
	hash = InitHash();	
	if ( hash == NULL )	{
		fprintf( stderr, "error: hash allocation error\n" );
		exit(1);
	}

	// �g�����̍쐬�i nCm, mC4 �j
	GenerateCombination( eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
	// �����I��L
	if ( eRmRedund ) {
		eNumCom2 = 17;
		LoadRmRedund();
	}

	// �Փˉ񐔃J�E���g
	for ( i = 0; i < doc_num; i++ ) {
		if ( i % 10000 == 0 )	fprintf( stderr, "%d/%d\n", i+1, doc_num );

		// �ߖT�\���̉��
		MakeNearsFromCentres( pss[i], nums[i] );

		if ( eThinPs ) {
			thin_nums[i] = SearchMinAreas( pss[i], areass[i], nums[i], thinids[i] );
			CalcAffineRotateOnceAndAddHash( pss[i], areass[i], i, thin_nums[i], disc, hash, thinids[i], collision );
		}
		else {
			for ( j = 0; j < nums[i]; j++ )	ids[j] = j;
			CalcAffineRotateOnceAndAddHash( pss[i], areass[i], i, nums[i], disc, hash, ids, collision );
			memset( ids, 0, nums[i] );
		}

		// ���Ƃ��܂�
		ClearNears( nums[i] );
	}

	// �g�����̉��
	ReleaseCombination();
	if ( eRmRedund )	eNumCom2 = CalcnCr( eGroup2Num, eGroup3Num );
	
	if ( !eExperimentMode )	fprintf( stderr, "Saving Hash Table...\n" );
	if ( eCompressHash )	SaveHashCompress( hash );
	//else					SaveHashText( hash );
	else					SaveHash( hash );
	if ( eThinPs ) {
		SaveThinFile( eDbDocs, thinids, pss, thin_nums, sizes, dbcors );
		//free( thin_nums );
		//for ( i = 0; i < 500; i++ )	free( thinids[i] );
		//free( thinids );
	} else {
		free( ids );
	}
	//*ptr_hash = hash;
}

void ConstructHash5( CvPoint **pss, double **areass, CvSize *sizes, int doc_num, int *nums, strDisc *disc, strHash ***ptr_hash, char **dbcors )
// �n�b�V�����\�z
{
	strHash **hash = NULL;
	int *collision=NULL;
	double ***features;
	int ft_length;
	int *bindex;
	char ***area_features;
	double *border, x, *fx;
	char hindex;
	int n=0, *id;

	int **thin_ids = NULL, *thin_nums = NULL;
	thin_ids = (int **)calloc( eDbDocs, sizeof(int *) );
	for ( int i = 0; i < eDbDocs; i++ )	thin_ids[i] = (int *)calloc( kMaxPointNum, sizeof(int) );
	thin_nums = (int *)calloc( eDbDocs, sizeof(int) );

	// �n�b�V���̏�����
	hash = InitHash();	
	if ( hash == NULL )	{
		fprintf( stderr, "error: hash allocation error\n" );
		exit(1);
	}

	// �g�����̍쐬�i nCm, mC4 �j
	GenerateCombination( eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
	// �����I��L
	if ( eRmRedund ) {
		eNumCom2 = 17;
		LoadRmRedund();
	}

	// �����ʊi�[�z��ifeatures[�����_][������][����]�j
	features = (double ***)calloc(kMaxPointNum, sizeof(double **));
	area_features = (char ***)calloc(kMaxPointNum, sizeof(char **));
	for ( int i = 0; i < kMaxPointNum; i++ ) {
		features[i] = (double **)calloc(eNumCom1-1, sizeof(double *));
		area_features[i] = (char **)calloc(eNumCom1-1, sizeof(char *));
		for ( int j = 0; j < eNumCom1-1; j++ ) {
			features[i][j] = (double *)calloc(eNumCom2, sizeof(double));
			area_features[i][j] = (char *)calloc(eNumCom2, sizeof(char));
		}
	}

	// �Փˉ񐔃J�E���g
	//for ( int i = 0; i < 1; i++ ) {
	for ( int i = 0; i < doc_num; i++ ) {
		if ( i % 10000 == 0 )	fprintf( stderr, "%d/%d\n", i+1, doc_num );
		// �ߖT�\���̉��
		MakeNearsFromCentres( pss[i], nums[i] );

		thin_nums[i] = SearchMinNears( pss[i], areass[i], nums[i], thin_ids[i] );
		//thin_nums[i] = SearchMinAreas( pss[i], areass[i], nums[i], thinids[i] );
		CalcAffineRotateOnce2( pss[i], areass[i], i, thin_nums[i], disc, hash, features, area_features, thin_ids[i] );


		//RankFeatureAndAddHash(features, area_features, i, nums[i], disc, areass[i], hash, 0, 0, NULL, NULL);

		// ���Ƃ��܂�
		ClearNears( nums[i] );
	}

	//RemoveListSameIndexAndQuotient(hash);

	// �g�����̉��
	ReleaseCombination();
	if ( eRmRedund )	eNumCom2 = CalcnCr( eGroup2Num, eGroup3Num );
	
	if ( !eExperimentMode )	fprintf( stderr, "Saving Hash Table...\n" );
	if ( eCompressHash )	SaveHashCompress( hash );
	else					SaveHash( hash );
	SaveThinFile( eDbDocs, thin_ids, pss, thin_nums, sizes, dbcors );
	// ���������
	for ( int i = 0; i < kMaxPointNum; i++ ) {
		for ( int j = 0; j < eNumCom1-1; j++ ) {
			free(features[i][j]);
			free(area_features[i][j]);
		}
		free(features[i]);
		free(area_features[i]);
	}
	free(features);
	free(area_features);
}

void ConstructHash2( CvPoint **pss, double **areass, CvSize *sizes, int doc_num, int *nums, strDisc *disc, strHash ***ptr_hash, char **dbcors )
// �n�b�V�����\�z
{
	strHash **hash = NULL;
	int *collision=NULL;
	double ***features;
	int ft_length;
	int *bindex;
	char ***area_features;
	double *border, x, *fx;
	char hindex;
	int n=0, *id;

	// �n�b�V���̏�����
	hash = InitHash();	
	if ( hash == NULL )	{
		fprintf( stderr, "error: hash allocation error\n" );
		exit(1);
	}

	// �g�����̍쐬�i nCm, mC4 �j
	GenerateCombination( eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
	// �����I��L
	if ( eRmRedund ) {
		eNumCom2 = 17;
		LoadRmRedund();
	}

	// �����ʊi�[�z��ifeatures[�����_][������][����]�j
	features = (double ***)calloc(kMaxPointNum, sizeof(double **));
	area_features = (char ***)calloc(kMaxPointNum, sizeof(char **));
	for ( int i = 0; i < kMaxPointNum; i++ ) {
		features[i] = (double **)calloc(eNumCom1-1, sizeof(double *));
		area_features[i] = (char **)calloc(eNumCom1-1, sizeof(char *));
		for ( int j = 0; j < eNumCom1-1; j++ ) {
			features[i][j] = (double *)calloc(eNumCom2, sizeof(double));
			area_features[i][j] = (char *)calloc(eNumCom2, sizeof(char));
		}
	}

	// �Փˉ񐔃J�E���g
	//for ( int i = 0; i < 1; i++ ) {
	for ( int i = 0; i < doc_num; i++ ) {
		if ( i % 10000 == 0 )	fprintf( stderr, "%d/%d\n", i+1, doc_num );
		// �ߖT�\���̉��
		MakeNearsFromCentres( pss[i], nums[i] );

		CalcAffineRotateOnce( pss[i], areass[i], i, nums[i], disc, hash, features, area_features );

		RankFeatureAndAddHash(features, area_features, i, nums[i], disc, areass[i], hash, 0, 0, NULL, NULL, pss[i]);

		// ���Ƃ��܂�
		ClearNears( nums[i] );
	}

	//RemoveListSameIndexAndQuotient(hash);

	// �g�����̉��
	ReleaseCombination();
	if ( eRmRedund )	eNumCom2 = CalcnCr( eGroup2Num, eGroup3Num );
	
	if ( !eExperimentMode )	fprintf( stderr, "Saving Hash Table...\n" );
	if ( eCompressHash )	SaveHashCompress( hash );
	else					SaveHash( hash );

	// ���������
	for ( int i = 0; i < kMaxPointNum; i++ ) {
		for ( int j = 0; j < eNumCom1-1; j++ ) {
			free(features[i][j]);
			free(area_features[i][j]);
		}
		free(features[i]);
		free(area_features[i]);
	}
	free(features);
	free(area_features);
}

void ConstructHash4( CvPoint **pss, double **areass, CvSize *sizes, int doc_num, int *nums, strDisc *disc, strHash ***ptr_hash, char **dbcors )
// �n�b�V�����\�z
{
	strHash **hash = NULL;
	int *collision=NULL;
	double ***features;
	int ft_length;
	int *bindex;
	char ***area_features;
	double *border, x, *fx;
	char hindex;
	int n=0, *id;

	// �n�b�V���̏�����
	hash = InitHash();	
	if ( hash == NULL )	{
		fprintf( stderr, "error: hash allocation error\n" );
		exit(1);
	}

	// �g�����̍쐬�i nCm, mC4 �j
	GenerateCombination( eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
	// �����I��L
	if ( eRmRedund ) {
		eNumCom2 = 17;
		LoadRmRedund();
	}

	// �����ʊi�[�z��ifeatures[�����_][������][����]�j
	features = (double ***)calloc(kMaxPointNum, sizeof(double **));
	area_features = (char ***)calloc(kMaxPointNum, sizeof(char **));
	for ( int i = 0; i < kMaxPointNum; i++ ) {
		features[i] = (double **)calloc(eNumCom1-1, sizeof(double *));
		area_features[i] = (char **)calloc(eNumCom1-1, sizeof(char *));
		for ( int j = 0; j < eNumCom1-1; j++ ) {
			features[i][j] = (double *)calloc(eNumCom2, sizeof(double));
			area_features[i][j] = (char *)calloc(eNumCom2, sizeof(char));
		}
	}

	// �Փˉ񐔃J�E���g
	//for ( int i = 0; i < 1; i++ ) {
	for ( int i = 0; i < doc_num; i++ ) {
		if ( !eExperimentMode )	fprintf( stderr, "%d/%d\n", i+1, doc_num );
		// �ߖT�\���̉��
		MakeNearsFromCentres( pss[i], nums[i] );

		CalcAffineRotateOnce( pss[i], areass[i], i, nums[i], disc, hash, features, area_features );

		//RankFeatureAndAddHash(features, area_features, i, nums[i], disc, areass[i], hash, 0, 0, NULL, NULL);

		// ���Ƃ��܂�
		ClearNears( nums[i] );
	}

	RemoveListSameIndexAndQuotient(hash);

	// �g�����̉��
	ReleaseCombination();
	if ( eRmRedund )	eNumCom2 = CalcnCr( eGroup2Num, eGroup3Num );
	
	if ( !eExperimentMode )	fprintf( stderr, "Saving Hash Table...\n" );
	if ( eCompressHash )	SaveHashCompress( hash );
	else					SaveHash( hash );

	// ���������
	for ( int i = 0; i < kMaxPointNum; i++ ) {
		for ( int j = 0; j < eNumCom1-1; j++ ) {
			free(features[i][j]);
			free(area_features[i][j]);
		}
		free(features[i]);
		free(area_features[i]);
	}
	free(features);
	free(area_features);
}

//void ConstructHash2( CvPoint **pss, double **areass, CvSize *sizes, int doc_num, int *nums, strDisc *disc, strHash ***ptr_hash, char **dbcors )
//// �n�b�V�����\�z
//{
//	int **thinids = NULL, *thin_nums = NULL, *ids = NULL;
//	strHash **hash = NULL;
//	int *collision=NULL;
//	double ***features;
//	int ft_length;
//	int *bindex;
//	char ***area_features;
//	double *border, x, *fx;
//	char hindex;
//	int n=0, *id;
//
//	// �n�b�V���̏�����
//	hash = InitHash();	
//	if ( hash == NULL )	{
//		fprintf( stderr, "error: hash allocation error\n" );
//		exit(1);
//	}
//
//	// �g�����̍쐬�i nCm, mC4 �j
//	GenerateCombination( eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
//	// �����I��L
//	if ( eRmRedund ) {
//		eNumCom2 = 17;
//		LoadRmRedund();
//	}	//printf("%d, %d\n", eNumCom1, eNumCom2);
//
//	// �����ʊi�[�z��ifeatures[�����_][������][����]�j
//	features = (double ***)calloc(kMaxPointNum, sizeof(double **));
//	for ( int i = 0; i < kMaxPointNum; i++ ) {
//		features[i] = (double **)calloc(eNumCom1-1, sizeof(double *));
//		for ( int j = 0; j < eNumCom1-1; j++ ) {
//			features[i][j] = (double *)calloc(eNumCom2, sizeof(double));
//		}
//	}
//
//
//	// �ʐϓ����ʊi�[�z��
//	if ( eUseArea ) {
//		area_features = (char ***)calloc(kMaxPointNum, sizeof(char **));
//		for ( int i = 0; i < kMaxPointNum; i++ ) {
//			area_features[i] = (char **)calloc(eNumCom1-1, sizeof(char *));
//			for ( int j = 0; j < eNumCom1-1; j++ ) {
//				area_features[i][j] = (char *)calloc(eNumCom2, sizeof(char));
//			}
//		}
//	}
//	// �e�̈�̓����_��
//	int tx = 4, ty = 5;
//	int **div_pnum = (int **)calloc( tx, sizeof(int*) );
//	for ( int i = 0; i < tx; i++ ) {
//		div_pnum[i] = (int *)calloc( ty, sizeof(int) );
//	}
//	// �e�̈�̓����_ID
//	int ***div_ps = (int ***)calloc( tx, sizeof(int*) );
//	for ( int i = 0; i < tx; i++ ) {
//		div_ps[i] = (int **)calloc( ty, sizeof(int*) );
//		for ( int j = 0; j < ty; j++ ) {
//			div_ps[i][j] = (int *)calloc( kMaxPointNum, sizeof(int) );
//		}
//	}
//
//	// �Փˉ񐔃J�E���g
//	//for ( int i = 0; i < 1; i++ ) {
//	for ( int i = 0; i < doc_num; i++ ) {
//		if ( !eExperimentMode )	fprintf( stderr, "%d/%d\n", i+1, doc_num );
//
//		// �ߖT�\���̉��
//		MakeNearsFromCentres( pss[i], nums[i] );
//
//		if ( eThinPs ) {
//			thin_nums[i] = SearchMinAreas( pss[i], areass[i], nums[i], thinids[i] );
//			CalcAffineRotateOnceAndAddHash( pss[i], areass[i], i, thin_nums[i], disc, hash, thinids[i], collision );
//		}
//		else {
//			CalcAffineRotateOnce( pss[i], areass[i], i, nums[i], disc, hash, features, area_features );
//		}
//
//		//printf("%d\n", nums[i]);
//		for ( int j = 0; j < nums[i]; j++ ) {	// �����_�̗̈�ւ̕���
//			//puts("b");
//			//printf("%3d: %3d, %3d\n", j, pss[i][j].x, pss[i][j].y);
//			//printf("%d, %d, %d, %d\n", sizes->width, sizes->height, pss[i][j].x, pss[i][j].y);
//			int div_x = (int)(((double)pss[i][j].x/(double)sizes->width)*tx);
//			int div_y = (int)(((double)pss[i][j].y/(double)sizes->height)*ty);
//			//printf("%d, %d, %d, %d\n", div_x, div_y, pss[i][j].x, pss[i][j].y);
//			//puts("c");
//			if(div_x < tx && div_y < ty )
//				div_ps[div_x][div_y][div_pnum[div_x][div_y]++] = j;
//			//puts("a");
//		}
//		//for ( int j = 0; j < 1; j++ ) {
//		//	for ( int k = 0; k < 1; k++ ) {
//		//		printf("%d\n", div_pnum[j][k]);
//		//		for ( int l = 0; l < div_pnum[j][k]; l++ ) {
//		//			printf("%d\n", div_ps[j][k][l]);
//		//		}
//		//	}
//		//}
//
//		RankFeatureAndAddHash3(features, area_features, i, nums[i], disc, areass[i], hash, tx, ty, div_ps, div_pnum);
//		//RankFeatureAndAddHash(features, area_features, i, nums[i], disc, areass[i], hash, tx, ty, div_ps, div_pnum);
//
//		// ���Ƃ��܂�
//		ClearNears( nums[i] );
//		for ( int j = 0; j < tx; j++ ) {
//			for ( int k = 0; k < ty; k++ ) {
//				div_pnum[j][k] = 0;
//			}
//		}
//	}
//
//	// �g�����̉��
//	ReleaseCombination();
//	if ( eRmRedund )	eNumCom2 = CalcnCr( eGroup2Num, eGroup3Num );
//	
//	if ( !eExperimentMode )	fprintf( stderr, "Saving Hash Table...\n" );
//	if ( eCompressHash )	SaveHashCompress( hash );
//	//else					SaveHashText( hash );
//	else					SaveHash( hash );
//
//	for ( int i = 0; i < kMaxPointNum; i++ ) {
//		for ( int j = 0; j < eNumCom1-1; j++ ) {
//			free(features[i][j]);
//		}
//		free(features[i]);
//	}
//	free(features);
//
//	for ( int i = 0; i < kMaxPointNum; i++ ) {
//		for ( int j = 0; j < eNumCom1-1; j++ ) {
//			free(area_features[i][j]);
//		}
//		free(area_features[i]);
//	}
//	free(area_features);
//
//	for ( int i = 0; i < tx; i++ ) {
//		free( div_pnum[i] );
//	}
//	free( div_pnum );
//	for ( int i = 0; i < tx; i++ ) {
//		for ( int j = 0; j < ty; j++ ) {
//			free( div_ps[i][j] );
//		}
//		free( div_ps[i] );
//	}
//	free( div_ps );
//}


void ConstructHash3( CvPoint **pss, double **areass, CvSize *sizes, int doc_num, int *nums, strDisc *disc, strHash ***ptr_hash, char **dbcors )
// �n�b�V�����\�z
{
	int **thinids = NULL, *thin_nums = NULL, *ids = NULL;
	strHash **hash = NULL;
	int *collision=NULL;
	double ***features;
	int ft_length;
	int *bindex;
	char ***area_features;
	double *border, x, *fx;
	char hindex;
	int n=0, *id;

	// �n�b�V���̏�����
	hash = InitHash();	
	if ( hash == NULL )	{
		fprintf( stderr, "error: hash allocation error\n" );
		exit(1);
	}

	// �g�����̍쐬�i nCm, mC4 �j
	GenerateCombination( eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
	if ( eRmRedund )	LoadRmRedund();	// �����I��L
	//printf("%d, %d\n", eNumCom1, eNumCom2);

	// �����ʊi�[�z��ifeatures[�����_][������][����]�j
	features = (double ***)calloc(kMaxPointNum, sizeof(double **));
	for ( int i = 0; i < kMaxPointNum; i++ ) {
		features[i] = (double **)calloc(eNumCom1-1, sizeof(double *));
		for ( int j = 0; j < eNumCom1-1; j++ ) {
			features[i][j] = (double *)calloc(eNumCom2, sizeof(double));
		}
	}

	// �ʐϓ����ʊi�[�z��
	if ( eUseArea ) {
		area_features = (char ***)calloc(kMaxPointNum, sizeof(char **));
		for ( int i = 0; i < kMaxPointNum; i++ ) {
			area_features[i] = (char **)calloc(eNumCom1-1, sizeof(char *));
			for ( int j = 0; j < eNumCom1-1; j++ ) {
				area_features[i][j] = (char *)calloc(eNumCom2, sizeof(char));
			}
		}
	}
	// �e�̈�̓����_��
	int tx = 4, ty = 5;
	int **div_pnum = (int **)calloc( tx, sizeof(int*) );
	for ( int i = 0; i < tx; i++ ) {
		div_pnum[i] = (int *)calloc( ty, sizeof(int) );
	}
	// �e�̈�̓����_ID
	int ***div_ps = (int ***)calloc( tx, sizeof(int*) );
	for ( int i = 0; i < tx; i++ ) {
		div_ps[i] = (int **)calloc( ty, sizeof(int*) );
		for ( int j = 0; j < ty; j++ ) {
			div_ps[i][j] = (int *)calloc( kMaxPointNum, sizeof(int) );
		}
	}

	int *spare_fnums = (int *)calloc(doc_num, sizeof(int));
	char ***spare_features = (char ***)calloc(doc_num, sizeof(char **));
	printf("doc_num : %d\n", sizeof(***spare_features));

	// �Փˉ񐔃J�E���g
	//for ( int i = 0; i < 1; i++ ) {
	for ( int i = 0; i < doc_num; i++ ) {
		if ( !eExperimentMode )	fprintf( stderr, "%d/%d\n", i+1, doc_num );

		// �ߖT�\���̉��
		MakeNearsFromCentres( pss[i], nums[i] );

		CalcAffineRotateOnce( pss[i], areass[i], i, nums[i], disc, hash, features, area_features );

		//printf("%d\n", nums[i]);
		for ( int j = 0; j < nums[i]; j++ ) {	// �����_�̗̈�ւ̕���
			//printf("%3d: %3d, %3d\n", j, pss[i][j].x, pss[i][j].y);
			//printf("%d, %d, %d, %d\n", sizes->width, sizes->height, pss[i][j].x, pss[i][j].y);
			int div_x = (int)(((double)pss[i][j].x/(double)sizes->width)*tx);
			int div_y = (int)(((double)pss[i][j].y/(double)sizes->height)*ty);
			//printf("%d, %d, %d, %d\n", div_x, div_y, pss[i][j].x, pss[i][j].y);
			if(div_x < tx && div_y < ty )
				div_ps[div_x][div_y][div_pnum[div_x][div_y]++] = j;
		}
		//for ( int j = 0; j < 1; j++ ) {
		//	for ( int k = 0; k < 1; k++ ) {
		//		printf("%d\n", div_pnum[j][k]);
		//		for ( int l = 0; l < div_pnum[j][k]; l++ ) {
		//			printf("%d\n", div_ps[j][k][l]);
		//		}
		//	}
		//}

		//RankFeatureAndAddHash(features, area_features, i, nums[i], disc, areass[i], hash, tx, ty, div_ps, div_pnum);
		//RankFeatureAndAddHash2(features, area_features, i, nums[i], disc, areass[i], hash, tx, ty, div_ps, div_pnum, spare_fnums, spare_features);

		// ���Ƃ��܂�
		ClearNears( nums[i] );
		for ( int j = 0; j < tx; j++ ) {
			for ( int k = 0; k < ty; k++ ) {
				div_pnum[j][k] = 0;
			}
		}
	}

	//RefineHash( hash, spare_fnums, spare_features );

	// �g�����̉��
	ReleaseCombination();
	
	if ( !eExperimentMode )	fprintf( stderr, "Saving Hash Table...\n" );
	if ( eCompressHash )	SaveHashCompress( hash );
	//else					SaveHashText( hash );
	else					SaveHash( hash );
	if ( eThinPs ) {
		SaveThinFile( eDbDocs, thinids, pss, thin_nums, sizes, dbcors );
		//free( thin_nums );
		//for ( i = 0; i < 500; i++ )	free( thinids[i] );
		//free( thinids );
	} else {
		free( ids );
	}
	//*ptr_hash = hash;

	for ( int i = 0; i < kMaxPointNum; i++ ) {
		for ( int j = 0; j < eNumCom1-1; j++ ) {
			free(features[i][j]);
		}
		free(features[i]);
	}
	free(features);

	for ( int i = 0; i < kMaxPointNum; i++ ) {
		for ( int j = 0; j < eNumCom1-1; j++ ) {
			free(area_features[i][j]);
		}
		free(area_features[i]);
	}
	free(area_features);

	for ( int i = 0; i < tx; i++ ) {
		free( div_pnum[i] );
	}
	free( div_pnum );
	for ( int i = 0; i < tx; i++ ) {
		for ( int j = 0; j < ty; j++ ) {
			free( div_ps[i][j] );
		}
		free( div_ps[i] );
	}
	free( div_ps );

}

void CombineDB( void )
// �n�b�V������
{
	int i, j, k;
	char hname[kMaxPathLen], pname[kMaxPathLen], cname[kMaxPathLen];
	strHash **hash=NULL;
	int doc_num1, doc_num2;
	CvPoint **reg_pss, **tmp_pss;
	CvSize *reg_sizes, *tmp_sizes;
	int *reg_nums, *tmp_nums;
	char **dbcors, **tmp_dbcors; 
	double **reg_areass, **tmp_areass;

	sprintf( cname, "%s%04d/%s", eCombineParentDir, eCombineStart, eConfigFileName );
	puts(cname);
	LoadConfigForCombine(cname);

	// �n�b�V���̏�����
	hash = InitHash();
	if ( hash == NULL ) {
		fprintf( stderr, "error: hash allocation error\n" );
		exit(1);
	}

	eDbDocs = 0;
	doc_num1 = 0;
	for ( i = eCombineStart; i < eCombineEnd; i++ ) {
		sprintf( hname, "%s%04d/%s", eCombineParentDir, i, eHashFileName );
		puts(hname);
		LoadHashCompressForCombine( hname, hash, eDbDocs );

		if (eThinPs)	sprintf( pname, "%s%04d/%s", eCombineParentDir, i, "thin_comp.dat" );
		else			sprintf( pname, "%s%04d/%s", eCombineParentDir, i, "point_comp.dat" );
		puts(pname);
		if (eThinPs)	doc_num2 = LoadThinFile2( pname, &tmp_dbcors );
		else			doc_num2 = LoadPointFile2( pname, &tmp_dbcors );
		eDbDocs += doc_num2;
		printf("%d\n", eDbDocs);

		if ( i == eCombineStart ) {
			dbcors = (char **)calloc( eDbDocs, sizeof(char *) );
			for ( j = 0; j < eDbDocs; j++ )
				dbcors[j] = (char *)calloc( kMaxPathLen, sizeof(char) );
		}
		else {
			dbcors = (char **)realloc( dbcors, sizeof(char *)*eDbDocs );
			for ( j = doc_num1; j < eDbDocs; j++ )
				dbcors[j] = (char *)calloc( kMaxPathLen, sizeof(char) );
		}

		// �ۑ�
		for ( j = 0; j < doc_num2; j++ ) {
			memcpy( dbcors[j+doc_num1], tmp_dbcors[j], kMaxPathLen );
		}
		doc_num1 = eDbDocs;

		// �|�C���^�Ɋi�[
		for ( j = 0; j < doc_num2; j++ ) 	free( tmp_dbcors[j] );
		free( tmp_dbcors );
	}

	// �n�b�V���ۑ�
	SaveHashCompress( hash );
	// �����_�t�@�C���̕ۑ��i�o�C�i���j
	if ( eThinPs )	SaveThinFile2( eDbDocs, dbcors );
	else			SavePointFile2( eDbDocs, dbcors );
	SaveConfig();	// �ݒ�t�@�C���̕ۑ�

	for ( i = 0; i < doc_num1; i++ )	free( dbcors[i] );
	free( dbcors );
}


//void CombineDB( void )
//// �n�b�V������
//{
//	int i, j, k;
//	char hname[kMaxPathLen], pname[kMaxPathLen];
//	strHash **hash=NULL;
//	int doc_num1, doc_num2;
//	CvPoint **reg_pss, **tmp_pss;
//	CvSize *reg_sizes, *tmp_sizes;
//	int *reg_nums, *tmp_nums;
//	char **dbcors, **tmp_dbcors; 
//
//	// �n�b�V���̏�����
//	hash = InitHash();
//	if ( hash == NULL ) {
//		fprintf( stderr, "error: hash allocation error\n" );
//		exit(1);
//	}
//	eDbDocs = 0;
//
//	// �x�[�X�n�b�V���̓ǂݍ���
//	sprintf( hname, "%s%s", eDataDir, eHashFileName );
//	puts(hname);
//	LoadHashCompressForCombine( hname, hash, eDbDocs );
//
//	// �x�[�X�|�C���g�̓ǂݍ���
//	sprintf( pname, "%s%s", eDataDir, eThinFileName );
//	puts(pname);
//	doc_num1 = LoadThinFile( pname, &reg_pss, &reg_sizes, &reg_nums, &dbcors );
//	eDbDocs = doc_num1;
//	printf("%d\n", eDbDocs);
//
//	for ( i = 1; i < 2; i++ ) {
//		sprintf( hname, "%s%s%02d/%s", ePFPrefix, "base", i, eHashFileName );
////		sprintf( hname, "%s%s", eDataDir, eHashDatFileName );
//		puts(hname);
//		LoadHashCompressForCombine( hname, hash, eDbDocs );
//
//		sprintf( pname, "%s%s%02d/%s", ePFPrefix, "base", i, eThinFileName );
////		sprintf( pname, "%s%s", eDataDir, eThinFileName );
//		puts(pname);
//		doc_num2 = LoadThinFile( pname, &tmp_pss, &tmp_sizes, &tmp_nums, &tmp_dbcors );
//		eDbDocs += doc_num2;
//		printf("%d\n", eDbDocs);
//
//		reg_pss = (CvPoint **)realloc( reg_pss, sizeof(CvPoint *)*eDbDocs );
//		reg_sizes = (CvSize *)realloc( reg_sizes, sizeof(CvSize)*eDbDocs );
//		reg_nums = (int *)realloc( reg_nums, sizeof(int)*eDbDocs );
//		dbcors = (char **)realloc( dbcors, sizeof(char *)*eDbDocs );
//		for ( j = doc_num1; j < eDbDocs; j++ )
//			dbcors[j] = (char *)calloc( kMaxPathLen, sizeof(char) );
//
//		// �ۑ�
//		for ( j = 0; j < doc_num2; j++ ) {
//			memcpy( dbcors[j+doc_num1], tmp_dbcors[j], kMaxPathLen );
//			reg_nums[j+doc_num1] = tmp_nums[j];
//			reg_sizes[j+doc_num1].width = tmp_sizes[j].width;
//			reg_sizes[j+doc_num1].height = tmp_sizes[j].height;
//			reg_pss[j+doc_num1] = (CvPoint *)calloc(reg_nums[j+doc_num1], sizeof(CvPoint));	// �����_�f�[�^�̃������m��
//			for ( k = 0; k < reg_nums[j+doc_num1]; k++ ) {
//				reg_pss[j+doc_num1][k].x = tmp_pss[j][k].x;
//				reg_pss[j+doc_num1][k].y = tmp_pss[j][k].y;
//			}
//		}
//		doc_num1 = eDbDocs;
//
//		// �|�C���^�Ɋi�[
//		for ( j = 0; j < doc_num2; j++ )	free( tmp_pss[j] );
//		free( tmp_pss );
//		free( tmp_sizes );
//		free( tmp_nums );
//		for ( j = 0; j < doc_num2; j++ ) 	free( tmp_dbcors[j] );
//		free( tmp_dbcors );
//	}
//
//	// �n�b�V���ۑ�
//	SaveHashCompress( hash );
//	// �����_�t�@�C���̕ۑ��i�o�C�i���j
//	SaveThinFileForCombine( eDbDocs, reg_pss, reg_sizes, reg_nums, dbcors );
//
//	for ( i = 0; i < doc_num1; i++ )	free( reg_pss[i] );
//	free( reg_pss );
//	free( reg_sizes );
//	free( reg_nums );
//	for ( i = 0; i < doc_num1; i++ )	free( dbcors[i] );
//	free( dbcors );
//}

void CombinePointFile( void )
// �n�b�V������
{
	int i, j, k;
	char pname[kMaxPathLen];
	int doc_num1, doc_num2;
	CvPoint **reg_pss, **tmp_pss;
	double **reg_areass, **tmp_areass;
	CvSize *reg_sizes, *tmp_sizes;
	int *reg_nums, *tmp_nums;
	char **dbcors, **tmp_dbcors; 

	eDbDocs = 0;

	// �x�[�X�|�C���g�̓ǂݍ���
	sprintf( pname, "%s%s", eDataDir, ePointFileName );
	puts(pname);
	doc_num1 = LoadPointFile( pname, &reg_pss, &reg_areass, &reg_sizes, &reg_nums, &dbcors );
	eDbDocs = doc_num1;
	printf("%d\n", eDbDocs);

	for ( i = eCombineStart; i < eCombineEnd; i++ ) {
		sprintf( pname, "%s%04d/%s", ePFPrefix, i, ePointFileName );
		//sprintf( pname, "%s%s%02d/%s", ePFPrefix, "base", i, ePointFileName );
//		sprintf( pname, "%s%s", eDataDir, eThinFileName );
		puts(pname);
		doc_num2 = LoadPointFile( pname, &tmp_pss, &tmp_areass, &tmp_sizes, &tmp_nums, &tmp_dbcors );
		eDbDocs += doc_num2;
		printf("%d\n", eDbDocs);

		reg_pss = (CvPoint **)realloc( reg_pss, sizeof(CvPoint *)*eDbDocs );
		reg_areass = (double **)realloc( reg_areass, sizeof(double *)*eDbDocs );
		reg_sizes = (CvSize *)realloc( reg_sizes, sizeof(CvSize)*eDbDocs );
		reg_nums = (int *)realloc( reg_nums, sizeof(int)*eDbDocs );
		dbcors = (char **)realloc( dbcors, sizeof(char *)*eDbDocs );
		for ( j = doc_num1; j < eDbDocs; j++ )
			dbcors[j] = (char *)calloc( kMaxPathLen, sizeof(char) );

		// �ۑ�
		for ( j = 0; j < doc_num2; j++ ) {
			memcpy( dbcors[j+doc_num1], tmp_dbcors[j], kMaxPathLen );
			reg_nums[j+doc_num1] = tmp_nums[j];
			reg_sizes[j+doc_num1].width = tmp_sizes[j].width;
			reg_sizes[j+doc_num1].height = tmp_sizes[j].height;
			reg_pss[j+doc_num1] = (CvPoint *)calloc(reg_nums[j+doc_num1], sizeof(CvPoint));	// �����_�f�[�^�̃������m��
			reg_areass[j+doc_num1] = (double *)calloc(reg_nums[j+doc_num1], sizeof(double));	// �����_�f�[�^�̃������m��
			for ( k = 0; k < reg_nums[j+doc_num1]; k++ ) {
				reg_pss[j+doc_num1][k].x = tmp_pss[j][k].x;
				reg_pss[j+doc_num1][k].y = tmp_pss[j][k].y;
				reg_areass[j+doc_num1][k] = tmp_areass[j][k];
			}
		}
		doc_num1 = eDbDocs;

		// �|�C���^�Ɋi�[
		for ( j = 0; j < doc_num2; j++ )	free( tmp_pss[j] );
		free( tmp_pss );
		for ( j = 0; j < doc_num2; j++ )	free( tmp_areass[j] );
		free( tmp_areass );
		free( tmp_nums );
		free( tmp_sizes );
		for ( j = 0; j < doc_num2; j++ )	free( tmp_dbcors[j] );
		free( tmp_dbcors );
	}

	// �����_�t�@�C���̕ۑ��i�o�C�i���j
	SavePointFile( eDbDocs, dbcors, reg_nums, reg_pss, reg_areass, reg_sizes );

	for ( i = 0; i < doc_num1; i++ )	free( reg_pss[i] );
	free( reg_pss );
	for ( i = 0; i < doc_num1; i++ )	free( reg_areass[i] );
	free( reg_areass );
	free( reg_sizes );
	free( reg_nums );
	for ( i = 0; i < doc_num1; i++ )	free( dbcors[i] );
	free( dbcors );
}

void CombineThinFile( void )
// �n�b�V������
{
	int i, j, k;
	char pname[kMaxPathLen];
	int doc_num1, doc_num2;
	CvPoint **reg_pss, **tmp_pss;
	CvSize *reg_sizes, *tmp_sizes;
	int *reg_nums, *tmp_nums;
	char **dbcors, **tmp_dbcors; 

	eDbDocs = 0;

	// �x�[�X�|�C���g�̓ǂݍ���
	sprintf( pname, "%s%s", eDataDir, eThinFileName );
	puts(pname);
	doc_num1 = LoadThinFile( pname, &reg_pss, &reg_sizes, &reg_nums, &dbcors );
	eDbDocs = doc_num1;
	printf("%d\n", eDbDocs);

	for ( i = 1; i < 2; i++ ) {
		sprintf( pname, "%s%s%02d/%s", ePFPrefix, "base", i, eThinFileName );
//		sprintf( pname, "%s%s", eDataDir, eThinFileName );
		puts(pname);
		doc_num2 = LoadThinFile( pname, &tmp_pss, &tmp_sizes, &tmp_nums, &tmp_dbcors );
		eDbDocs += doc_num2;
		printf("%d\n", eDbDocs);

		reg_pss = (CvPoint **)realloc( reg_pss, sizeof(CvPoint *)*eDbDocs );
		reg_sizes = (CvSize *)realloc( reg_sizes, sizeof(CvSize)*eDbDocs );
		reg_nums = (int *)realloc( reg_nums, sizeof(int)*eDbDocs );
		dbcors = (char **)realloc( dbcors, sizeof(char *)*eDbDocs );
		for ( j = doc_num1; j < eDbDocs; j++ )
			dbcors[j] = (char *)calloc( kMaxPathLen, sizeof(char) );

		// �ۑ�
		for ( j = 0; j < doc_num2; j++ ) {
			memcpy( dbcors[j+doc_num1], tmp_dbcors[j], kMaxPathLen );
			reg_nums[j+doc_num1] = tmp_nums[j];
			reg_sizes[j+doc_num1].width = tmp_sizes[j].width;
			reg_sizes[j+doc_num1].height = tmp_sizes[j].height;
			reg_pss[j+doc_num1] = (CvPoint *)calloc(reg_nums[j+doc_num1], sizeof(CvPoint));	// �����_�f�[�^�̃������m��
			for ( k = 0; k < reg_nums[j+doc_num1]; k++ ) {
				reg_pss[j+doc_num1][k].x = tmp_pss[j][k].x;
				reg_pss[j+doc_num1][k].y = tmp_pss[j][k].y;
			}
		}
		doc_num1 = eDbDocs;

		// �|�C���^�Ɋi�[
		for ( j = 0; j < doc_num2; j++ )	free( tmp_pss[j] );
		free( tmp_pss );
		free( tmp_nums );
		for ( j = 0; j < doc_num2; j++ )	free( tmp_dbcors[j] );
		free( tmp_dbcors );
	}

	// �����_�t�@�C���̕ۑ��i�o�C�i���j
	SaveThinFileForCombine( eDbDocs, reg_pss, reg_sizes, reg_nums, dbcors );

	for ( i = 0; i < doc_num1; i++ )	free( reg_pss[i] );
	free( reg_pss );
	free( reg_sizes );
	free( reg_nums );
	for ( i = 0; i < doc_num1; i++ ) 	free( dbcors[i] );
	free( dbcors );
}

void CombineHash( void )
// �n�b�V������
{
	int i, doc_num;
	char hname[kMaxPathLen], pname[kMaxPathLen];
	strHash **hash=NULL;

	// �n�b�V���̏�����
	hash = InitHash();
	if ( hash == NULL ) {
		fprintf( stderr, "error: hash allocation error\n" );
		exit(1);
	}
	eDbDocs = 0;

	// �x�[�X�n�b�V���̓ǂݍ���
	sprintf( hname, "%s%s", eDataDir, eHashFileName );
	puts(hname);
	LoadHashCompressForCombine( hname, hash, eDbDocs );

	// �x�[�X�|�C���g�̓ǂݍ���
	sprintf( pname, "%s%s", eDataDir, ePointFileName );
	puts(pname);
	doc_num = LoadDocNum( pname );
	eDbDocs = doc_num;
	printf("%d\n", eDbDocs);

	for ( i = 1; i < 2; i++ ) {
		sprintf( hname, "%s%s%02d/%s", ePFPrefix, "base", i, eHashFileName );
//		sprintf( hname, "%s%s", eDataDir, eHashDatFileName );
		puts(hname);
		LoadHashCompressForCombine( hname, hash, eDbDocs );

		sprintf( pname, "%s%s%02d/%s", ePFPrefix, "base", i, ePointFileName );
//		sprintf( pname, "%s%s", eDataDir, eThinFileName );
		puts(pname);
		doc_num = LoadDocNum( pname );
		eDbDocs += doc_num;
		printf("%d\n", eDbDocs);
	}

	// �n�b�V���ۑ�
	SaveHashCompress( hash );
}
