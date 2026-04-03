#include "def_general.h"

/* dcams.c */
#define	GLOBAL_DEFINE	/* extern�ϐ��ɂ����Ŏ��̂�^���� */

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>	// min,max�p
#include <math.h>
#include <iostream>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
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
// �l�b�g���[�N
#ifdef	WIN32
#include <winsock2.h>	// socket
#include "ws_cl.h"
#else
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "sock_sv.h"
#endif

#include "extern.h"
#include "init.h"
#include "disc.h"
#include "hash.h"
#include "gencomb.h"
#include "nears.h"
#include "procimg.h"
#include "fpath.h"
#include "retrieve.h"
#include "constdb.h"
#include "load.h"
#include "save.h"
#include "camserver.h"
#include "outputimg.h"

#define CLR
#define DRAW_COR_ALLa

using namespace std;

extern int fnum;
extern int mnum;
extern int dnum;

int main( int argc, char **argv )
{
	int i, j, args, num, res, argi=0, *score=NULL, ret_time = 0;
	int total_que=0, suc_que=0, total_time=0, sum=0, ans=0;
	char **dbcors=NULL;
	strDisc disc;
	strHash **hash;
	double **reg_areass=NULL, *areas=NULL;
	double total_diff = 0.0L, total_top = 0.0L;
	cv::Point **reg_pss=NULL, *ps=NULL;
	cv::Size *reg_sizes=NULL, size;
	cv::Mat img;
	int *reg_nums=NULL;

	int *index;
	double *cr, a, x, fx;
	char hindex;
#ifdef CLRa
	cv::Mat con_img, orig_img, fp_img;
#endif
#ifdef DRAW_COR_ALL
	int pcornum=0;
	int pcor[kMaxPointNum][2];
#endif
	char line[kMaxPathLen];
	FILE *fp;
	int random[1000];

	// �������s�������̊i�[
	vector<string> failure;

	// �l�b�g���[�Nini�t�@�C���ǂݍ���
	ReadIniFile();
	// ������͏���
	if ( ( argi = AnalyzeArgAndSetExtern( argc, argv ) ) < 0 )	return 1;
	unsigned long long int count=0;
	unsigned long long int value=0;
	switch ( eEntireMode ) {
		/******************* �ʏ팟�����[�h *********************/
	case RETRIEVE_MODE:
		fprintf( stderr, "Retrieval from static image\n");
		// �ݒ�t�@�C���̓ǂݍ���
		LoadConfig();
		// ���U���t�@�C���̓ǂݍ���
		LoadDisc( &disc );

		// �����_�t�@�C���̓ǂݍ���
		if ( eThinPs )	eDbDocs = LoadThinFile2( NULL, &dbcors );								// �T���v�����O�L��
		//if ( eThinPs )	eDbDocs = LoadThinFile( NULL, &reg_pss, &reg_sizes, &reg_nums, &dbcors );								// �T���v�����O�L��
		else			eDbDocs = LoadPointFile2( NULL, &dbcors );	// �T���v�����O����

		// �n�b�V���̓ǂݍ���
		//LoadHashCompress( &hash );
		LoadHashCompressToStorage( &hash );


		//if ( (fp = fopen("rand.txt", "r")) == NULL ) {
		//	return 0;
		//}

		//for ( int i = 0; i < 1000; i++ ) {
		//	fgets( line, kDiscLineLen, fp );
		//	sscanf( line, "%u", &random[i] );
		//	printf( "%s\n", dbcors[random[i]] );
		//}
		//fclose(fp);


		//else			eDbDocs = LoadPointFile( NULL, &reg_pss, &reg_areass, &reg_sizes, &reg_nums, &dbcors );	// �T���v�����O����

		CalculateAverageCollision(hash);
		//RemoveListSameIndexAndQuotient(hash);
		//CalculateAverageCollision(hash);


		// �ߖT�\���ۑ��p�������m��
		SecureNears();
		// ���[�e�[�u���m��
		score = (int *)calloc(eDbDocs, sizeof(int));
		// �g�ݍ��킹�̍쐬
		GenerateCombination( eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
		if ( eRmRedund ) {
			eNumCom2 = 17;
			LoadRmRedund();	// �����I��L
		}
		// �����J�n
		args = argi;
		//for ( i = 0; i < 1; i++ ) {
		for ( eMultiNum = 1; eMultiNum < 17; eMultiNum+=2 ) {
			argi = args;
			//printf("collision : %d\n", eMaxHashCollision);
			//total_time = 0;
			//total_diff = 0;
			//total_top = 0;
			//suc_que = 0;
			//total_que = 0;
			//sum = 0;
			fnum = 0;
			mnum = 0;
			dnum = 0;

			for ( ; argi < argc; argi++ ) {
				if ( !eExperimentMode ) 
					fprintf( stdout, "%s\n", argv[argi] );
				// �����_�̒��o
				if ( IsDat( argv[argi] ) ) {
					num = LoadQueryPointFile( argv[argi], &ps, &areas );
				}
				else {
					// �����摜�̐���
					img = GetConnectedImage( argv[argi] );
					if ( img.empty() )	continue;
					// �����_���o�E�A�������̖ʐόv�Z
					num = MakeFeaturePoint( img, &ps, &areas, &size );	
#ifdef CLRa
					con_img = img.clone();
					orig_img = cv::imread( argv[argi], cv::IMREAD_GRAYSCALE );
					fp_img = MakeFeaturePointImage( orig_img, con_img, ps, num );
					OutPutImage( fp_img );
					//cvReleaseImage( &orig_img );
					//cvReleaseImage( &con_img );
					//cvReleaseImage( &fp_img );
#endif
					//cvReleaseImage( &img );
				}
				// �ߖT�\���̉��
				MakeNearsFromCentres( ps, num );

#ifdef DRAW_COR_ALL
				//�Ή��_�`��
				res = RetrieveCor( ps, areas, num, score, pcor, &pcornum, &disc, reg_nums, hash );
				//res = 596+808;
				//res = 3198;
				// size����������
				DrawCor( ps, num, reg_sizes[res], res, reg_pss[res], reg_nums[res], reg_sizes[res], pcor, pcornum );
#else
				// ����
				res = Retrieve( ps, areas, num, score, &disc, reg_nums, &ret_time, hash );
				//printf("%d\n", res);
#endif
				// ��������
				if ( !eExperimentMode ) 
					printf( "%s : %d\n\n", dbcors[res], score[res] );
				// ���Ƃ��܂�
				free( ps );
				free( areas );
				ClearNears( num );

				// ���x�E�������Ԃ̏���
				if ( eExperimentMode ) {	// �������[�h�Ȃ琸�x�v��
					total_que++;
					if ( IsSucceed( argv[argi], dbcors[res] ) )	suc_que++;
					else {
						failure.push_back(argv[argi]);
						fprintf( stdout, "%s\n", argv[argi] );
						printf( "%s : %d\n", dbcors[res], score[res] );
					}
					total_time += ret_time;
					total_diff += Calc12Diff( score );
					total_top += (double)score[res];
					for ( j = 0; j < eDbDocs; j++ ) {
						if ( score[j] > 0 )	sum += score[j];
					}
					//printf("%d\n", score[596+k]);
					//ans += score[596+argi+10000000-1];
				}
			}

			//eMaxHashCollision -= 50;
			//value = 0; 
			//count = 0;
			//for ( unsigned long long int ii = 0; ii < eHashSize; ii++ ) {
			//	if ( hash[ii] != NULL ) {
			//		//printf("%d\n", *hash[i]);
			//		if ( *hash[ii] >= eMaxHashCollision ) {
			//			*hash[ii] = 0;
			//		}
			//		else {
			//			count++;
			//			value+=*hash[ii];
			//		}
			//	}
			//}
			//printf("%lf\n", (double)value/(double)count);
			// ��������
			if ( eExperimentMode ) {	// �������[�h�Ȃ猋�ʏo��
				printf("Accuracy : %d/%d(%.2f)\n", suc_que, total_que, ((double)suc_que)/((double)total_que)*100.0 );
				printf("Average Proc Time : %d micro sec\n", (int)(((double)total_time) / ((double)total_que)) );
				printf("Average Top Vote : %.2f\n", (total_top / (double)total_que) );
				printf("Average Diff : %.2f\n", (total_diff / (double)total_que) );
				printf( "%d\n", sum );

				//for ( int k = 0; k < failure.size(); k++ ) {
				//	cout << failure[k] << endl;
				//}
			}

		}
		free( score );
		//ReleaseHash( hash );
		ReleaseNears();

		// ��������
		if ( eExperimentMode ) {	// �������[�h�Ȃ猋�ʏo��
			printf("Accuracy : %d/%d(%.2f)\n", suc_que, total_que, ((double)suc_que)/((double)total_que)*100.0 );
			printf("Average Proc Time : %d micro sec\n", (int)(((double)total_time) / ((double)total_que)) );
			printf("Average Top Vote : %.2f\n", (total_top / (double)total_que) );
			printf("Average Diff : %.2f\n", (total_diff / (double)total_que) );
			printf( "%d\n", sum );
		}

		break;
		/************************ USB�J�������[�h ************************/
	case USBCAM_SERVER_MODE:
		RetrieveUSBCamServer();
		break;
		//		/************************ �X�}�[�g�t�H���A�g���[�h ************************/
		//		case SMART_SERVER_MODE:
		//			RetrieveSmartphoneServer();
		//			break;
		/********************** �n�b�V���\�z���[�h **********************/
	case CONST_HASH_MODE:
		if ( !eExperimentMode )	fprintf( stderr, "Hash Constoraction\n" );

		// �����_�t�@�C���̍쐬
		if ( !eExperimentMode )	fprintf( stderr, "Extracting Feature Points...\n" );
		CreatePointFile( &reg_pss, &reg_areass, &reg_sizes, &reg_nums, &dbcors );
const_rest:
		// �ߖT�\���i�[�z��̊m��
		SecureNears();

		if ( eLoadDiscHash ) {
			// ���U���t�@�C���̓ǂݍ���
			if ( !eExperimentMode )	fprintf( stderr, "Load Disc File... \n" );
			LoadDisc( &disc );
		} else {
			// ���U���t�@�C���̍쐬
			if ( !eExperimentMode )	fprintf( stderr, "Make Disc File... \n" );
			MakeDiscFile( min( eDocNumForMakeDisc, eDbDocs ), reg_pss, reg_nums, &disc );
		}

		// �n�b�V���̍\�z
		if ( !eExperimentMode )	fprintf( stderr, "Make Affine File... \n" );
		//if ( eReHash )			ConstructHash3( reg_pss, reg_areass, reg_sizes, eDbDocs, reg_nums, &disc, &hash, dbcors );
		if ( eReHash )			ConstructHash4( reg_pss, reg_areass, reg_sizes, eDbDocs, reg_nums, &disc, &hash, dbcors );
		else if ( eSampling )	ConstructHash5( reg_pss, reg_areass, reg_sizes, eDbDocs, reg_nums, &disc, &hash, dbcors );
		else if ( eThresh )		ConstructHash2( reg_pss, reg_areass, reg_sizes, eDbDocs, reg_nums, &disc, &hash, dbcors );
		else					ConstructHash( reg_pss, reg_areass, reg_sizes, eDbDocs, reg_nums, &disc, &hash, dbcors );


		// �ݒ�t�@�C���̕ۑ�
		if ( !eExperimentMode )	fprintf( stderr, "Save Config File...\n" );
		SaveConfig();

		break;
		/******************* point.dat����̃n�b�V���\�z���[�h ************************/
	case CONST_HASH_PF_MODE:
		if ( !eExperimentMode ) fprintf( stderr, "Hash Construction using point.dat\n" );

		eDbDocs = LoadPointFile( NULL, &reg_pss, &reg_areass, &reg_sizes, &reg_nums, &dbcors );
		//printf("%d\n", reg_nums[0]);
		//printf("%s\n", dbcors[0]);
		//printf("%s, %d, %d\n", dbcors[4], reg_pss[4][6].x, reg_pss[4][6].y);
		//printf("%s, %d, %d\n", dbcors[5], reg_pss[5][6].x, reg_pss[5][6].y);
		//printf("%s, %d, %d\n", dbcors[7], reg_pss[7][4].x, reg_pss[7][4].y);
		//printf("%s, %d, %d\n", dbcors[7], reg_pss[7][6].x, reg_pss[7][6].y);
		//for ( int j = 0; j < reg_nums[0]; j++ ) {
		//	printf("%3d: %3d, %3d\n", j, reg_pss[0][j].x, reg_pss[0][j].y);
		//}
		//printf("%d\n", eDbDocs);
		goto const_rest;

		break;
		/******************* �����_�t�@�C���쐬���[�h ***********************/
	case CONST_POINT_MODE:
		if ( !eExperimentMode )	fprintf( stderr, "Hash Constoraction\n" );

		// �����_�t�@�C���̍쐬
		if ( !eExperimentMode )	fprintf( stderr, "Extracting Feature Points...\n" );
		CreatePointFile( &reg_pss, &reg_areass, &reg_sizes, &reg_nums, &dbcors );
		break;
		/************************ DB�������[�h ************************/
	case COMBINE_DB_MODE:
		fprintf( stderr, "Combine Point File\n" );
		//LoadConfig();	// �ݒ�t�@�C���̓ǂݍ���
		CombineDB();	// DB�̌���
		break;
		/************************ �����_�t�@�C���������[�h ************************/
	case COMBINE_PF_MODE:
		fprintf( stderr, "Combine Point File\n" );
		CombinePointFile();	// point.dat�̌���
		//fprintf( stderr, "Combine Thin File\n" );
		//CombineThinFile();	// thin.dat�̌���
		break;
		/************************ �����_�t�@�C���������[�h ************************/
	case COMPRESS_PF_MODE:
		fprintf( stderr, "Compress Point File\n" );
		LoadConfig();	// �ݒ�t�@�C���̓ǂݍ���
		if ( eThinPs ) {
			eDbDocs = LoadThinFile( NULL, &reg_pss, &reg_sizes, &reg_nums, &dbcors );
			SaveThinFile2( eDbDocs, dbcors );
		} else {
			eDbDocs = LoadPointFile( NULL, &reg_pss, &reg_areass, &reg_sizes, &reg_nums, &dbcors );
			SavePointFile2( eDbDocs, dbcors );
		}
		break;
		/************************ �n�b�V���������[�h ************************/
		//case COMBINE_HASH_MODE:
		//	fprintf( stderr, "Combine Hash\n" );
		//	LoadConfig();	// �ݒ�t�@�C���̓ǂݍ���
		//	CombineHash();	// �n�b�V���̌���
		//	SaveConfig();	// �ݒ�t�@�C���̕ۑ�
		//	break;
		/************************* �N�G�������_���[�h ***********************/
	case MAKE_QUERY_DAT_MODE:
		for ( ; argi < argc; argi++ ) {
			if ( !eExperimentMode ) fprintf( stdout, "%s\n", argv[argi] );

			// �����摜�̐���
			img = GetConnectedImage( argv[argi] );
			if ( img.empty() )	continue;
			// �����_���o�E�A�������̖ʐόv�Z
			num = MakeFeaturePoint( img, &(ps), &(areas), &(size));
#ifdef CLRa
			con_img = img.clone();
			orig_img = cv::imread( argv[argi], cv::IMREAD_GRAYSCALE );
			fp_img = MakeFeaturePointImage( orig_img, con_img, ps, num );
			OutPutImage( fp_img );
			//cvReleaseImage( &orig_img );
			//cvReleaseImage( &con_img );
			//cvReleaseImage( &fp_img );
#endif
			//cvReleaseImage( &img );

			SaveQueryPointFile( argv[argi], num, ps, areas );

			// ���Ƃ��܂�
			free( ps );
			free( areas );
		}
		return 0;
	default:
		break;
	}
	return 0;
}

