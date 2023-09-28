#include "def_general.h"

/* dcams.c */
#define	GLOBAL_DEFINE	/* extern変数にここで実体を与える */

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>	// min,max用
#include <math.h>
#include <iostream>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
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
// ネットワーク
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
	CvPoint **reg_pss=NULL, *ps=NULL;
	CvSize *reg_sizes=NULL, size;
	IplImage *img;
	int *reg_nums=NULL;

	int *index;
	double *cr, a, x, fx;
	char hindex;
#ifdef CLRa
	IplImage *con_img, *orig_img, *fp_img;
#endif
#ifdef DRAW_COR_ALL
	int pcornum=0;
	int pcor[kMaxPointNum][2];
#endif
	char line[kMaxPathLen];
	FILE *fp;
	int random[1000];

	// 検索失敗文書名の格納
	vector<string> failure;

	// ネットワークiniファイル読み込み
	ReadIniFile();
	// 引数解析処理
	if ( ( argi = AnalyzeArgAndSetExtern( argc, argv ) ) < 0 )	return 1;
	unsigned long long int count=0;
	unsigned long long int value=0;
	switch ( eEntireMode ) {
		/******************* 通常検索モード *********************/
	case RETRIEVE_MODE:
		fprintf( stderr, "Retrieval from static image\n");
		// 設定ファイルの読み込み
		LoadConfig();
		// 離散化ファイルの読み込み
		LoadDisc( &disc );

		// 特徴点ファイルの読み込み
		if ( eThinPs )	eDbDocs = LoadThinFile2( NULL, &dbcors );								// サンプリング有り
		//if ( eThinPs )	eDbDocs = LoadThinFile( NULL, &reg_pss, &reg_sizes, &reg_nums, &dbcors );								// サンプリング有り
		else			eDbDocs = LoadPointFile2( NULL, &dbcors );	// サンプリング無し

		// ハッシュの読み込み
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


		//else			eDbDocs = LoadPointFile( NULL, &reg_pss, &reg_areass, &reg_sizes, &reg_nums, &dbcors );	// サンプリング無し

		CalculateAverageCollision(hash);
		//RemoveListSameIndexAndQuotient(hash);
		//CalculateAverageCollision(hash);


		// 近傍構造保存用メモリ確保
		SecureNears();
		// 得票テーブル確保
		score = (int *)calloc(eDbDocs, sizeof(int));
		// 組み合わせの作成
		GenerateCombination( eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
		if ( eRmRedund ) {
			eNumCom2 = 17;
			LoadRmRedund();	// 特徴選択有
		}
		// 検索開始
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
				// 特徴点の抽出
				if ( IsDat( argv[argi] ) ) {
					num = LoadQueryPointFile( argv[argi], &ps, &areas );
				}
				else {
					// 結合画像の生成
					img = GetConnectedImage( argv[argi] );
					if ( img == NULL )	continue;
					// 特徴点抽出・連結成分の面積計算
					num = MakeFeaturePoint( img, &ps, &areas, &size );	
#ifdef CLRa
					con_img = cvCloneImage( img );
					orig_img = cvLoadImage( argv[argi], 0 );
					fp_img = MakeFeaturePointImage( orig_img, con_img, ps, num );
					OutPutImage( fp_img );
					cvReleaseImage( &orig_img );
					cvReleaseImage( &con_img );
					cvReleaseImage( &fp_img );
#endif
					cvReleaseImage( &img );
				}
				// 近傍構造の解析
				MakeNearsFromCentres( ps, num );

#ifdef DRAW_COR_ALL
				//対応点描画
				res = RetrieveCor( ps, areas, num, score, pcor, &pcornum, &disc, reg_nums, hash );
				//res = 596+808;
				//res = 3198;
				// sizeがおかしい
				DrawCor( ps, num, reg_sizes[res], res, reg_pss[res], reg_nums[res], reg_sizes[res], pcor, pcornum );
#else
				// 検索
				res = Retrieve( ps, areas, num, score, &disc, reg_nums, &ret_time, hash );
				//printf("%d\n", res);
#endif
				// 検索結果
				if ( !eExperimentMode ) 
					printf( "%s : %d\n\n", dbcors[res], score[res] );
				// あとしまつ
				free( ps );
				free( areas );
				ClearNears( num );

				// 精度・処理時間の処理
				if ( eExperimentMode ) {	// 実験モードなら精度計測
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
			// 実験結果
			if ( eExperimentMode ) {	// 実験モードなら結果出力
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

		// 実験結果
		if ( eExperimentMode ) {	// 実験モードなら結果出力
			printf("Accuracy : %d/%d(%.2f)\n", suc_que, total_que, ((double)suc_que)/((double)total_que)*100.0 );
			printf("Average Proc Time : %d micro sec\n", (int)(((double)total_time) / ((double)total_que)) );
			printf("Average Top Vote : %.2f\n", (total_top / (double)total_que) );
			printf("Average Diff : %.2f\n", (total_diff / (double)total_que) );
			printf( "%d\n", sum );
		}

		break;
		/************************ USBカメラモード ************************/
	case USBCAM_SERVER_MODE:
		RetrieveUSBCamServer();
		break;
		//		/************************ スマートフォン連携モード ************************/
		//		case SMART_SERVER_MODE:
		//			RetrieveSmartphoneServer();
		//			break;
		/********************** ハッシュ構築モード **********************/
	case CONST_HASH_MODE:
		if ( !eExperimentMode )	fprintf( stderr, "Hash Constoraction\n" );

		// 特徴点ファイルの作成
		if ( !eExperimentMode )	fprintf( stderr, "Extracting Feature Points...\n" );
		CreatePointFile( &reg_pss, &reg_areass, &reg_sizes, &reg_nums, &dbcors );
const_rest:
		// 近傍構造格納配列の確保
		SecureNears();

		if ( eLoadDiscHash ) {
			// 離散化ファイルの読み込み
			if ( !eExperimentMode )	fprintf( stderr, "Load Disc File... \n" );
			LoadDisc( &disc );
		} else {
			// 離散化ファイルの作成
			if ( !eExperimentMode )	fprintf( stderr, "Make Disc File... \n" );
			MakeDiscFile( min( eDocNumForMakeDisc, eDbDocs ), reg_pss, reg_nums, &disc );
		}

		// ハッシュの構築
		if ( !eExperimentMode )	fprintf( stderr, "Make Affine File... \n" );
		//if ( eReHash )			ConstructHash3( reg_pss, reg_areass, reg_sizes, eDbDocs, reg_nums, &disc, &hash, dbcors );
		if ( eReHash )			ConstructHash4( reg_pss, reg_areass, reg_sizes, eDbDocs, reg_nums, &disc, &hash, dbcors );
		else if ( eSampling )	ConstructHash5( reg_pss, reg_areass, reg_sizes, eDbDocs, reg_nums, &disc, &hash, dbcors );
		else if ( eThresh )		ConstructHash2( reg_pss, reg_areass, reg_sizes, eDbDocs, reg_nums, &disc, &hash, dbcors );
		else					ConstructHash( reg_pss, reg_areass, reg_sizes, eDbDocs, reg_nums, &disc, &hash, dbcors );


		// 設定ファイルの保存
		if ( !eExperimentMode )	fprintf( stderr, "Save Config File...\n" );
		SaveConfig();

		break;
		/******************* point.datからのハッシュ構築モード ************************/
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
		/******************* 特徴点ファイル作成モード ***********************/
	case CONST_POINT_MODE:
		if ( !eExperimentMode )	fprintf( stderr, "Hash Constoraction\n" );

		// 特徴点ファイルの作成
		if ( !eExperimentMode )	fprintf( stderr, "Extracting Feature Points...\n" );
		CreatePointFile( &reg_pss, &reg_areass, &reg_sizes, &reg_nums, &dbcors );
		break;
		/************************ DB結合モード ************************/
	case COMBINE_DB_MODE:
		fprintf( stderr, "Combine Point File\n" );
		//LoadConfig();	// 設定ファイルの読み込み
		CombineDB();	// DBの結合
		break;
		/************************ 特徴点ファイル結合モード ************************/
	case COMBINE_PF_MODE:
		fprintf( stderr, "Combine Point File\n" );
		CombinePointFile();	// point.datの結合
		//fprintf( stderr, "Combine Thin File\n" );
		//CombineThinFile();	// thin.datの結合
		break;
		/************************ 特徴点ファイル結合モード ************************/
	case COMPRESS_PF_MODE:
		fprintf( stderr, "Compress Point File\n" );
		LoadConfig();	// 設定ファイルの読み込み
		if ( eThinPs ) {
			eDbDocs = LoadThinFile( NULL, &reg_pss, &reg_sizes, &reg_nums, &dbcors );
			SaveThinFile2( eDbDocs, dbcors );
		} else {
			eDbDocs = LoadPointFile( NULL, &reg_pss, &reg_areass, &reg_sizes, &reg_nums, &dbcors );
			SavePointFile2( eDbDocs, dbcors );
		}
		break;
		/************************ ハッシュ結合モード ************************/
		//case COMBINE_HASH_MODE:
		//	fprintf( stderr, "Combine Hash\n" );
		//	LoadConfig();	// 設定ファイルの読み込み
		//	CombineHash();	// ハッシュの結合
		//	SaveConfig();	// 設定ファイルの保存
		//	break;
		/************************* クエリ特徴点モード ***********************/
	case MAKE_QUERY_DAT_MODE:
		for ( ; argi < argc; argi++ ) {
			if ( !eExperimentMode ) fprintf( stdout, "%s\n", argv[argi] );

			// 結合画像の生成
			img = GetConnectedImage( argv[argi] );
			if ( img == NULL )	continue;
			// 特徴点抽出・連結成分の面積計算
			num = MakeFeaturePoint( img, &(ps), &(areas), &(size));
#ifdef CLRa
			con_img = cvCloneImage( img );
			orig_img = cvLoadImage( argv[argi], 0 );
			fp_img = MakeFeaturePointImage( orig_img, con_img, ps, num );
			OutPutImage( fp_img );
			cvReleaseImage( &orig_img );
			cvReleaseImage( &con_img );
			cvReleaseImage( &fp_img );
#endif
			cvReleaseImage( &img );

			SaveQueryPointFile( argv[argi], num, ps, areas );

			// あとしまつ
			free( ps );
			free( areas );
		}
		return 0;
	default:
		break;
	}
	return 0;
}

