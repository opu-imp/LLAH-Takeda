#include "def_general.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <opencv2/opencv.hpp> 
#ifdef _DEBUG
#pragma comment( lib, "opencv_imgproc230d.lib" )
#pragma comment( lib, "opencv_core230d.lib" )
#pragma comment( lib, "opencv_highgui230d.lib" )
#else
#pragma comment( lib, "opencv_imgproc230.lib" )
#pragma comment( lib, "opencv_core230.lib" )
#pragma comment( lib, "opencv_highgui230.lib" )
#endif
using namespace cv;
using namespace std;

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
#include "camserver.h"
#include "load.h"
#include "nears.h"
#include "gencomb.h"
#include "retrieve.h"
#include "proj4p.h"
#include "projrecov.h"
#include "fpath.h"

extern int **nears;

void RetrieveUSBCamServer( void )
	// USBカメラサーバモード
{
	int num, res=0, ret, *score = NULL, *reg_nums=NULL/*, *thin_nums*/;
	int pcornum=0;
	int pcor[kMaxPointNum][2];
	char doc_name[kMaxPathLen], **dbcors = NULL, doc_reject[kMaxPathLen]="reject";
	CvPoint *ps=NULL, **reg_pss=NULL;
	CvSize img_size, *reg_sizes=NULL, res_size;
	double *areas=NULL, **reg_areass=NULL;
	strDisc disc;
	strHash **hash;

	strProjParam param, zero_param;
	float tranceParam1[16], tranceParam2[16];
	SOCKET sid1=0, sid2=0, sidpt=0, sidres=0;
	struct sockaddr_in addr;

	// 設定ファイルの読み込み
	LoadConfig();
	// 離散化ファイルの読み込み
	LoadDisc( &disc );
	// ハッシュの読み込み
	LoadHashCompressToStorage( &hash );
	// 特徴点ファイルの読み込み
	if ( eThinPs )	eDbDocs = LoadThinFile( NULL, &reg_pss, &reg_sizes, &reg_nums, &dbcors );								// サンプリング有り
	else			eDbDocs = LoadPointFile( NULL, &reg_pss, &reg_areass, &reg_sizes, &reg_nums, &dbcors );	// サンプリング無し
	// 近傍構造保存用メモリ確保
	SecureNears();
	// 得票テーブル確保
	score = (int *)calloc(eDbDocs, sizeof(int));
	// 組み合わせの作成
	GenerateCombination( eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
	if ( eRmRedund )	LoadRmRedund();	// 特徴選択有
	// ネットワーク関係
start_nego2:
	puts("Ready");
#ifdef WIN32
	if ( ( sid1 = InitWinSockSvTCP( eTCPPort, "10.127.1.178") ) < 0 )	exit(1);	// socketの初期化
	if ( ( sid2 = AcceptWinSockSvTCP( sid1 ) ) < 0 )	exit(1);	// accept
#else
	if ( ( sid1 = InitSockSvTCP( eTCPPort, eServerName ) ) < 0 )	exit(1);	// socketの初期化
	if ( ( sid2 = AcceptSockSvTCP( sid1 ) ) < 0 )	exit(1);	// accept
#endif

	zero_param.a1 = 0.0; zero_param.a2 = 0.0; zero_param.a3 = 0.0; zero_param.b1 = 0.0; zero_param.b2 = 0.0; zero_param.b3 = 0.0; zero_param.c1 = 0.0; zero_param.c2 = 0.0;
	param = zero_param;
	res_size = reg_sizes[0];
	strcpy( doc_name, doc_reject );
	pcornum=0;
	res=0;

	//ret = SendDoc( sid2, "test", &addr, eProtocol );
	//ret = SendResultParam( sid2, doc_name, kMaxDocNameLen, &param, &res_size, &addr, eProtocol );
	ret = SendResultParam3D( sid2, doc_name, kMaxDocNameLen, &param, tranceParam1, tranceParam2, &res_size, &addr, eProtocol );

	for ( ; ; ) {
		num = RecvPointsAreas( sid2, &ps, &areas, &img_size );	// 特徴点データを受信
		if ( num < 0 )	break;	// 通信失敗

		if ( num >= kMinPoints ) {	// 特徴点数が最小値以上
			MakeNearsFromCentresDiv( ps, num, &img_size, kDivX, kDivY, eGroup1Num );	// 近傍点計算（分割版）
			res = RetrieveCor( ps, areas, num, score, pcor, &pcornum, &disc, reg_nums, hash );
			// 最小対応点数以上なら、射影変換パラメータを計算
			if ( pcornum >= kMinPointsToCalcParam ) CalcProjParamTop( ps, reg_pss[res], pcor, pcornum, &param, PROJ_REVERSE, PARAM_RANSAC );	// 登録画像に上書き
			else	param = zero_param;

			// 計算時間の表示
			//			OutPutResultSv( score, end-start, 0/*5*/, doc_name );
			if ( score[res] > kTopThr ) {	// トップの得票数が閾値以下（リジェクト）
				GetBasename( dbcors[res], kMaxPathLen, doc_name );
			}
			else {
				strcpy( doc_name, doc_reject );
				param = zero_param;	// 検索結果がないのにパラメータだけあるってのも変なので
				pcornum=0;
			}
			puts( doc_name );
		}
		else {	// 特徴点数が最小値を満たさない
			//puts("min");
			strcpy( doc_name, doc_reject );
			param = zero_param;
			pcornum=0;
		}
		if ( strcmp( doc_name, doc_reject ) )	res_size = reg_sizes[res];	// 検索成功ならそのサイズ
		else	res_size = reg_sizes[0];	// そうでなければデータベースの0番のサイズ

		if ( memcmp(&param, &zero_param, sizeof(strProjParam))!=0 ) {
			ExtractRAndTFromP2(param, tranceParam1);
			// 変換行列の表示
			for ( int i = 0; i < 4; i++ ) {
				for ( int j = 0; j < 4; j++ ) {
					printf("%lf\t", tranceParam1[i+j*4]);
				}
				puts("");
			}
			//puts("");
			//for ( int i = 0; i < 4; i++ ) {
			//	for ( int j = 0; j < 4; j++ ) {
			//		printf("%lf\t", tranceParam2[i+j*4]);
			//	}
			//	puts("");
			//}
			//puts("");
		}

		//ret = SendResultParam( sid2, doc_name, kMaxDocNameLen, &param, &res_size, &addr, eProtocol );
		ret = SendResultParam3D( sid2, doc_name, kMaxDocNameLen, &param, tranceParam1, tranceParam2, &res_size, &addr, eProtocol );


		if ( ps != NULL && num > 0 )	free( ps );	// psの解放
		if ( areas != NULL && num > 0 )	free( areas );
		ClearNears( num );	// nearsの解放
	}
	// 終了処理
	//free(score);

#ifdef	WIN32
	CloseWinSock( sid2 );
	CloseWinSock( sid1 );
#else
	ShutdownSockSvTCP( sid2 );	// shutdown
	CloseSock( sid1 );	// close
#endif

	goto start_nego2;	// ネゴ待ちまで戻る
}

void normalize ( Vec3f&  vec )
{
	float a = vec[0] * vec[0];
	float b = vec[1] * vec[1];
	float c = vec[2] * vec[2];

	float norm = sqrt ( a + b + c );

	vec[0] /= norm;
	vec[1] /= norm;
	vec[2] /= norm;
}
void ExtractRAndTFromP2(strProjParam param, float *tranceParam)
{
	double data_H[3][3] = {{param.a1, param.b1, param.c1}, {param.a2, param.b2, param.c2}, {param.a3, param.b3, 1.0}};	// ホモグラフィ行列
//	double data_A[3][3] = {{569.65435791, 0.0, 321.98031616}, {0.0, 568.74615479, 237.28897095}, {0.0, 0.0, 1.0}};		// カメラ内部パラメータ
	double data_A[3][3] = {{2.51902783e+003, 0.0, 1.29164893e+003}, {0.0, 2.51729639e+003, 9.62208679e+002}, {0.0, 0.0, 1.0}};		// カメラ内部パラメータ
	
	Mat HL(3, 3, CV_64F, data_H);	// ホモグラフィ行列
	Mat A(3, 3, CV_64F, data_A);	// カメラ内部パラメータ

	Mat RT = A.inv() * HL;

	Vec3f rVec1 = Vec3f( RT.at< double >(0,0), RT.at< double >(1,0), RT.at< double >(2,0) );
	Vec3f rVec2 = Vec3f( RT.at< double >(0,1), RT.at< double >(1,1), RT.at< double >(2,1) );
	Vec3f tVec  = Vec3f( RT.at< double >(0,2), RT.at< double >(1,2), RT.at< double >(2,2) );

	Vec3f rVec3 = rVec1.cross ( rVec2 );

	normalize ( rVec1 );
	normalize ( rVec2 );
	normalize ( rVec3 );

	for ( int i = 0; i < 3; i++ ) {
		tranceParam[0+i] = rVec1[i];
		tranceParam[4+i] = rVec2[i];
		tranceParam[8+i] = rVec3[i];
		tranceParam[12+i] = tVec[i];
		cout << rVec1[i] << "\t";
		cout << rVec2[i] << "\t";
		cout << rVec3[i] << "\t";
		cout << tVec[i] << endl;
	}
	puts("");

	tranceParam[3] = 0.0;
	tranceParam[7] = 0.0;
	tranceParam[11] = 0.0;
	tranceParam[15] = 1.0;
}



void ExtractRAndTFromP(strProjParam param, float *tranceParam1, float *tranceParam2)
// 射影変換パラメータから回転行列Rと併進行列Tを抽出する
{
	double data_A[] = {param.a1, param.b1, param.c1, param.a2, param.b2, param.c2, param.a3, param.b3, 1.0};
	// ホモグラフィ行列
	CvMat *Hl			= cvCreateMat(3, 3, CV_64F);	// ホモグラフィ行列
	CvMat *Hl_tenchi	= cvCreateMat(3, 3, CV_64F);	// ホモグラフィ行列の転置行列
	CvMat *HlHl			= cvCreateMat(3, 3, CV_64F);	// ホモグラフィの積
	CvMat *eigenVec		= cvCreateMat(3, 3, CV_64F);	// 固有ベクトル（行に格納）
	CvMat *eigenVal		= cvCreateMat(3, 1, CV_64F);	// 固有値（降順）
	CvMat *v1			= cvCreateMat(3, 1, CV_64F);	// 第1固有ベクトル
	CvMat *v2			= cvCreateMat(3, 1, CV_64F);	// 第2固有ベクトル
	CvMat *v3			= cvCreateMat(3, 1, CV_64F);	// 第3固有ベクトル
	CvMat *u1			= cvCreateMat(3, 1, CV_64F);	
	CvMat *u2			= cvCreateMat(3, 1, CV_64F);
	CvMat *v2u1			= cvCreateMat(3, 1, CV_64F);	// v2とu1の外積
	CvMat *v2u2			= cvCreateMat(3, 1, CV_64F);	// v2とu2の外積
	CvMat *U1			= cvCreateMat(3, 3, CV_64F);	// 行列(v2, u1, v2u1)
	CvMat *U2			= cvCreateMat(3, 3, CV_64F);	// 行列(v2, u2, v2u2)
	CvMat *H			= cvCreateMat(3, 3, CV_64F);	// 真のホモグラフィ行列
	CvMat *Hv2			= cvCreateMat(3, 1, CV_64F);
	CvMat *Hu1			= cvCreateMat(3, 1, CV_64F);
	CvMat *Hu2			= cvCreateMat(3, 1, CV_64F);
	CvMat *Hv2Hu1		= cvCreateMat(3, 1, CV_64F);
	CvMat *Hv2Hu2		= cvCreateMat(3, 1, CV_64F);
	CvMat *W1			= cvCreateMat(3, 3, CV_64F);	// 行列(Hv2, Hu1, Hv2Hu1)
	CvMat *W2			= cvCreateMat(3, 3, CV_64F);	// 行列(Hv2, Hu2, Hv2Hu2)
	CvMat *U1tenchi		= cvCreateMat(3, 3, CV_64F);
	CvMat *R1			= cvCreateMat(3, 3, CV_64F);
	CvMat *T1tmp		= cvCreateMat(3, 3, CV_64F);
	CvMat *T1			= cvCreateMat(3, 1, CV_64F);
	CvMat *U2tenchi		= cvCreateMat(3, 3, CV_64F);
	CvMat *R2			= cvCreateMat(3, 3, CV_64F);
	CvMat *T2tmp		= cvCreateMat(3, 3, CV_64F);
	CvMat *T2			= cvCreateMat(3, 1, CV_64F);
	double sigma1=0.0, sigma2=0.0, sigma3=0.0;	// 固有値

	// 射影変換パラメータを行列に格納
	for ( int i = 0; i < 3; i++ ) {
		for ( int j = 0; j < 3; j++ ) 
			cvmSet(Hl, i, j, data_A[i*3+j]);
	}

	// ホモグラフィ行列の転置行列
	cvTranspose(Hl, Hl_tenchi);
	// Hl_tenchi*Hl
	cvMatMul(Hl_tenchi, Hl, HlHl);
	// HlHlの固有値分解
	cvEigenVV(HlHl, eigenVec, eigenVal, 0.0, -1, -1);

	// 行列から固有ベクトルの抽出
	for ( int i = 0; i < 3; i++ ) {
		cvmSet(v1, i, 0, cvmGet(eigenVec, i, 0) );
		cvmSet(v2, i, 0, cvmGet(eigenVec, i, 1) );
		cvmSet(v3, i, 0, cvmGet(eigenVec, i, 2) );
	}

	//for ( int i = 0; i < 3; i++ ) {
	//	for ( int j = 0; j < 3; j++ ) {
	//		printf("%lf\t", cvmGet(eigenVec, i, j));
	//	}
	//	puts("");
	//}
	//puts("");
	puts("eigen");
	for ( int i = 0; i < 3; i++ ) {
		printf("%lf\t", cvmGet(eigenVal, i, 0));
	}
	puts("");
	puts("v1");
	for ( int i = 0; i < 3; i++ ) {
		printf("%lf\t", cvmGet(v1, i, 0));
	}
	puts("");
	puts("v2");
	for ( int i = 0; i < 3; i++ ) {
		printf("%lf\t", cvmGet(v2, i, 0));
	}
	puts("");
	puts("v3");
	for ( int i = 0; i < 3; i++ ) {
		printf("%lf\t", cvmGet(v3, i, 0));
	}
	puts("");
	puts("");
	// ベクトルから固有値の抽出（第2固有値で正規化）
	sigma2=cvmGet(eigenVal,1,0);
	sigma1=cvmGet(eigenVal,0,0)/sigma2;
	sigma3=cvmGet(eigenVal,2,0)/sigma2;

	CvMat *tmp1 = cvCreateMat(3, 1, CV_64F);
	CvMat *tmp2 = cvCreateMat(3, 1, CV_64F);

	// u1の計算
	cvAddWeighted( v1, sqrt(1-sigma3), v1, 0, 0, tmp1);
	cvAddWeighted( v3, sqrt(sigma1-1), v3, 0, 0, tmp2);
	cvAdd(tmp1, tmp2, u1);
	cvAddWeighted( u1, 1.0/sqrt(sigma1-sigma3), u1, 0, 0, u1);

	// u2の計算
	cvSub(tmp1, tmp2, u2);
	cvAddWeighted( u2, 1.0/sqrt(sigma1-sigma3), u2, 0, 0, u2);

	// 外積の計算
	cvCrossProduct(v2,u1,v2u1);
	cvCrossProduct(v2,u2,v2u2);
	
	for ( int i = 0; i < 3; i++ ) {
		// U1(v2,u1,v2u1)
		cvmSet(U1,i,0,cvmGet(v2, i, 0));
		cvmSet(U1,i,1,cvmGet(u1, i, 0));
		cvmSet(U1,i,2,cvmGet(v2u1, i, 0));
		
		// U1(v2,u2,v2u2)
		cvmSet(U2,i,0,cvmGet(v2, i, 0));
		cvmSet(U2,i,1,cvmGet(u2, i, 0));
		cvmSet(U2,i,2,cvmGet(v2u2, i, 0));
	}
	double dv2 = 0.0;
	for ( int i = 0; i < 3; i++ ) {
		dv2 += cvmGet(v2, i, 0) *cvmGet(v2, i, 0);
	}
	printf("v2 : %lf\n", sqrt(dv2));
	puts("v2 : ");
	for ( int i = 0; i < 3; i++ ) {
		printf("%lf\t", cvmGet(v2, i, 0));
	}
	puts("");
	double du = cvDet(U1);
	printf("U1 : %lf\n", du);
	//puts("u1");
	//for ( int i = 0; i < 3; i++ ) {
	//	for ( int j = 0; j < 3; j++ ) {
	//		printf("%lf\t", cvmget(u1, i, j));
	//	}
	//	puts("");
	//}
	//puts("");
	//for ( int i = 0; i < 3; i++ ) {
	//	for ( int j = 0; j < 3; j++ ) {
	//		printf("%lf\t", cvmGet(U2, i, j));
	//	}
	//	puts("");
	//}
	//puts("");
	// 真のホモグラフィ行列（Hlの正規化？）
	cvAddWeighted( Hl, 1.0/sqrt(sigma2), Hl, 0, 0, H);

	CvMat *H_tenchi = cvCreateMat(3, 3, CV_64F);
	CvMat *HH		= cvCreateMat(3, 3, CV_64F);
	// ホモグラフィ行列の転置行列
	cvTranspose(H, H_tenchi);
	// Hl_tenchi*Hl
	cvMatMul(H_tenchi, H, HH);
	// HlHlの固有値分解
	cvEigenVV(HH, eigenVec, eigenVal, 0.0, -1, -1);

	printf("v2 : %lf\n", cvmGet(eigenVal,1,0));

	puts("H : ");
	for ( int i = 0; i < 3; i++ ) {
		for ( int j = 0; j < 3; j++ ) {
			printf("%lf\t", cvmGet(H, i, j));
		}
		puts("");
	}
	puts("");
	
	
	double dh = cvDet(H);
	printf("H : %lf\n", dh);

	cvMatMul(H, v2, Hv2);
	cvMatMul(H, u1, Hu1);
	cvMatMul(H, u2, Hu2);
	cvCrossProduct(Hv2,Hu1,Hv2Hu1);
	cvCrossProduct(Hv2,Hu2,Hv2Hu2);

	puts("Hv2");
	for ( int i = 0; i < 3; i++ ) {
			printf("%lf\t", cvmGet(Hv2, i, 0));
	}
	puts("");

	puts("Hu1");
	for ( int i = 0; i < 3; i++ ) {
			printf("%lf\t", cvmGet(Hu1, i, 0));
	}
	puts("");

	puts("Hv2Hu1");
	for ( int i = 0; i < 3; i++ ) {
			printf("%lf\t", cvmGet(Hv2Hu1, i, 0));
	}
	puts("");

	double dHv2 = 0.0;
	for ( int i = 0; i < 3; i++ ) {
		dHv2 += cvmGet(Hv2, i, 0) *cvmGet(Hv2, i, 0);
	}
	printf("Hv2 : %lf\n", sqrt(dHv2));

	double dHu1 = 0.0;
	for ( int i = 0; i < 3; i++ ) {
		dHu1 += cvmGet(Hu1, i, 0) *cvmGet(Hu1, i, 0);
	}
	printf("Hu1 : %lf\n", sqrt(dHu1));

	double dHu2 = 0.0;
	for ( int i = 0; i < 3; i++ ) {
		dHu2 += cvmGet(Hu2, i, 0) *cvmGet(Hu2, i, 0);
	}
	printf("Hu2 : %lf\n", sqrt(dHu2));

	double dHv2Hu1 = 0.0;
	for ( int i = 0; i < 3; i++ ) {
		dHv2Hu1 += cvmGet(Hv2Hu1, i, 0) *cvmGet(Hv2Hu1, i, 0);
	}
	printf("Hv2Hu1 : %lf\n", sqrt(dHv2Hu1));

	double dHv2Hu2 = 0.0;
	for ( int i = 0; i < 3; i++ ) {
		dHv2Hu2 += cvmGet(Hv2Hu2, i, 0) *cvmGet(Hv2Hu2, i, 0);
	}
	printf("Hv2Hu2 : %lf\n", sqrt(dHv2Hu2));

	for ( int i = 0; i < 3; i++ ) {
		// (Hv2, Hu1, Hv2Hu1)
		cvmSet(W1,i,0,cvmGet(Hv2, i, 0));
		cvmSet(W1,i,1,cvmGet(Hu1, i, 0));
		cvmSet(W1,i,2,cvmGet(Hv2Hu1, i, 0));

		// (Hv2, Hu2, Hv2Hu2)
		cvmSet(W2,i,0,cvmGet(Hv2, i, 0));
		cvmSet(W2,i,1,cvmGet(Hu2, i, 0));
		cvmSet(W2,i,2,cvmGet(Hv2Hu2, i, 0));
	}
	//puts("W1 : ");

	for ( int i = 0; i < 3; i++ ) {
		for ( int j = 0; j < 3; j++ ) {
			printf("%lf\t", cvmGet(W1, i, j));
		}
		puts("");
	}
	puts("");
	double dw = cvDet(W1);
	printf("W1 : %lf\n", dw);
	// 真のホモグラフィ行列（Hlの正規化？）
	cvAddWeighted( W1, 1.0/pow(dw, 1.0/3.0), W1, 0, 0, W1);
	dw = cvDet(W1);
	printf("W1 : %lf\n", dw);

	for ( int i = 0; i < 3; i++ ) {
		for ( int j = 0; j < 3; j++ ) {
			printf("%lf\t", cvmGet(W1, i, j));
		}
		puts("");
	}
	puts("");
	double dw2 = cvDet(W2);
	printf("W2 : %lf\n", dw2);
	// 真のホモグラフィ行列（Hlの正規化？）
	cvAddWeighted( W2, 1.0/dw2, W2, 0, 0, W2);
	dw2 = cvDet(W2);
	printf("W2 : %lf\n", dw2);
	for ( int i = 0; i < 3; i++ ) {
		for ( int j = 0; j < 3; j++ ) {
			printf("%lf\t", cvmGet(W1, i, j));
		}
		puts("");
	}
	puts("");
	//puts("W2 : ");
	//for ( int i = 0; i < 3; i++ ) {
	//	for ( int j = 0; j < 3; j++ ) {
	//		printf("%lf\t", cvmGet(W2, i, j));
	//	}
	//	puts("");
	//}
	//puts("");
	// R1,T1の抽出（R,Tは2つできる）

	cvTranspose(U1, U1tenchi);
	// R1 = W1U1
	cvMatMul(W1, U1tenchi, R1);
	// T1 = (H-R1)N1
	cvSub(H, R1, T1tmp);
	cvMatMul(T1tmp, v2u1, T1);

	// R2,T2の抽出（R,Tは2つできる）
	cvTranspose(U2, U2tenchi);
	// R2 = W2U2
	cvMatMul(W2, U2tenchi, R2);
	// T2 = (H-R2)N2
	cvSub(H, R2, T2tmp);
	cvMatMul(T1tmp, v2u2, T2);

	// 配列に格納
	for ( int i = 0; i < 3; i++ ) {
		tranceParam1[0+4*i] = (float)cvmGet(R1,0,i);
		tranceParam1[1+4*i] = (float)cvmGet(R1,1,i);
		tranceParam1[2+4*i] = (float)cvmGet(R1,2,i);
	}
	for ( int i = 0; i < 3; i++ ) {
		tranceParam1[12+i] = (float)cvmGet(T1,i,0);
	}
	tranceParam1[3] = 0.0;
	tranceParam1[7] = 0.0;
	tranceParam1[11] = 0.0;
	tranceParam1[15] = 1.0;

	// 配列に格納
	for ( int i = 0; i < 3; i++ ) {
		tranceParam2[0+4*i] = (float)cvmGet(R2,0,i);
		tranceParam2[1+4*i] = (float)cvmGet(R2,1,i);
		tranceParam2[2+4*i] = (float)cvmGet(R2,2,i);
	}
	for ( int i = 0; i < 3; i++ ) {
		tranceParam2[12+i] = (float)cvmGet(T2,i,0);
	}
	tranceParam2[3] = 0.0;
	tranceParam2[7] = 0.0;
	tranceParam2[11] = 0.0;
	tranceParam2[15] = 1.0;

	////R,Tの表示
	//puts("R1 :\t\t\t\t\t\tT1 :");
	//for ( int i = 0; i < 3; i++ ) {
	//	for ( int j = 0; j < 3; j++ ) {
	//		printf("%lf\t", cvmGet(R1, i, j));
	//	}
	//	printf("%lf\n", cvmGet(T1, i, 0));
	//}
	//puts("");


	// Hの復元
	CvMat *N1tenchi = cvCreateMat(1,3,CV_64F);
	CvMat *_H = cvCreateMat(3,3,CV_64F);

	cvTranspose(v2u1, N1tenchi);
	cvMatMul(T1, N1tenchi, _H);
	cvAdd(R1, _H, _H);

	//for ( int i = 0; i < 3; i++ ) {
	//	for ( int j = 0; j < 3; j++ ) {
	//		printf("%lf\t", cvmGet(H, i, j));
	//	}
	//	puts("");
	//}
	//puts("");
	//for ( int i = 0; i < 3; i++ ) {
	//	for ( int j = 0; j < 3; j++ ) {
	//		printf("%lf\t", cvmGet(_H, i, j));
	//	}
	//	puts("");
	//}

}


int RecvComSetting( SOCKET sid, int *ptc, int *pt_port, int *res_port, char *cl_name )
	// 通信設定を受信
{
	int ret;

	ret = recv( sid, (char *)ptc, sizeof(int), RECV_FLAG );	// プロトコル
	if ( ret <= 0 )	return -1;
	ret = recv( sid, (char *)pt_port, sizeof(int), RECV_FLAG );	// 特徴点ポート
	if ( ret <= 0 )	return -1;
	ret = recv( sid, (char *)res_port, sizeof(int), RECV_FLAG );	// 検索結果ポート
	if ( ret <= 0 )	return -1;
	ret = recv( sid, cl_name, kMaxNameLen, RECV_FLAG );	// クライアント名
	if ( ret <= 0 )	return -1;

	return 1;
}

int SendResultParam( SOCKET sid, char *doc_name, int len, strProjParam *param, CvSize *img_size, struct sockaddr_in *addr, int ptc )
	// 文書名と射影変換パラメータを送信
{
	int ret;
	char buff[kSendBuffSize];

	memcpy( buff, doc_name, kMaxDocNameLen );
	memcpy( buff + kMaxDocNameLen, param, sizeof(strProjParam) );
	memcpy( buff + kMaxDocNameLen + sizeof(strProjParam), img_size, sizeof(CvSize) );

	ret = send ( sid, buff, kSendBuffSize, SEND_FLAG );

	return ret;
}

int SendResultParam3D( SOCKET sid, char *doc_name, int len, strProjParam *param, float *tranceParam1, float *tranceParam2, CvSize *img_size, struct sockaddr_in *addr, int ptc )
	// 文書名と射影変換パラメータを送信
{
	int ret;
	char buff[kSendBuffSize3D];

	memcpy( buff, doc_name, kMaxDocNameLen );
	memcpy( buff + kMaxDocNameLen, param, sizeof(strProjParam) );
	memcpy( buff + kMaxDocNameLen + sizeof(strProjParam), img_size, sizeof(CvSize) );
	memcpy( buff + kMaxDocNameLen + sizeof(strProjParam) + sizeof(CvSize), tranceParam1, sizeof(float)*16 );

	ret = send ( sid, buff, kSendBuffSize3D, SEND_FLAG );

	return ret;
}

int SendResultCor( SOCKET sid, char *doc_name, int len, strProjParam *param, CvSize size, int pcor[][2], int pcornum, CvPoint *ps, CvPoint *psall, struct sockaddr_in *addr, int ptc )
	// 文書名と対応点を送信
{
	int i, buff_cur=0, ret;
	char buff[kSendBuffSizeCor];
	CvPoint corps[kMaxPointNum][2];

	puts(doc_name);

	memcpy( buff, doc_name, kMaxDocNameLen );
	buff_cur += kMaxDocNameLen;
	memcpy( buff + buff_cur, param, sizeof(strProjParam) );
	buff_cur += sizeof(strProjParam);
	memcpy( buff + buff_cur, &size, sizeof(CvSize) );
	buff_cur += sizeof(CvSize);
	memcpy( buff + buff_cur, &pcornum, sizeof(int) );
	buff_cur += sizeof(int);
	for ( i = 0; i < pcornum; i++ ) {
		memcpy( buff + buff_cur, &(ps[pcor[i][0]]), sizeof(CvPoint) );
		buff_cur += 8;
	}
	for ( i = 0; i < pcornum; i++ ) {
		memcpy( buff + buff_cur, &(psall[pcor[i][1]]), sizeof(CvPoint) );
		buff_cur += 8;
	}

	if ( ptc == kTCP ) 
		ret = send( sid, buff, kSendBuffSizeCor, SEND_FLAG );
	else
		ret = sendto( sid, buff, kSendBuffSizeCor, SEND_FLAG, (struct sockaddr *)addr, sizeof(*addr) );

	return ret;
}

int RecvPointsAreas( SOCKET sid, CvPoint **ps0, double **areas0, CvSize *size )
	// 点データを受信
{
	int i, num = 0, ret=0, buff_cur, total=0;
	char buff[kRecvBuffSizeAreas];
	CvPoint *ps;
	double *areas;
	unsigned short areas_us[kMaxPointNum];

	// バッファに受信
	while (total < 25012) {
		ret = recv( sid, &buff[total], 25012-total, RECV_FLAG );
		total += ret;
	}
	//printf("%d\n", ret);
	if ( ret <= 0 )	return -1;	// 受信失敗
	// バッファから画像サイズを得る
	for ( i = 0, buff_cur = 0; i < (int)sizeof(CvSize); i++, buff_cur++ ) {
		((char *)size)[i] = buff[buff_cur];
	}
	//printf("%d\n",sizeof(CvSize));
	//printf("Mat size: %d, %d\n", (*size).width, (*size).height);
	// バッファから特徴点数を得る
	for ( i = 0; i < (int)sizeof(int); i++, buff_cur++ ) {
		((char *)&num)[i] = buff[buff_cur];
	}
	//printf("Point Num: %d\n", num);
	// 特徴点数が1以上ならメモリを確保
	if ( num > 0 )	{
		ps = (CvPoint *)calloc( num, sizeof(CvPoint) );
		areas = (double *)calloc( num, sizeof(double) );
	}
	else {
		ps = NULL;
		areas = NULL;
	}
	*ps0 = ps;
	*areas0 = areas;
	// バッファから特徴点データを得る
	for ( i = 0; i < (int)(sizeof(CvPoint) * num); i++, buff_cur++ ) {
		((char *)ps)[i] = buff[buff_cur];
	}
	//for ( i = 0; i < num; i++ )	printf("Point: %d, %d\n", ps[i].x, ps[i].y);

	// バッファから面積データを得る
	buff_cur = sizeof(CvSize) + sizeof(int) + sizeof(CvPoint) * kMaxPointNum;
	for ( i = 0; i < (int)(sizeof(unsigned short) * num); i++, buff_cur++ ) {
		((char *)areas_us)[i] = buff[buff_cur];
	}
	for ( i = 0; i < num; i++ ) {
		areas[i] = (double)areas_us[i];
		//printf("%lf\n", areas[i]);
	}

	return num;
}


int RecvFlag( SOCKET sid )
	// 開始フラグをを受信
{
	int i, num = 0, ret, buff_cur;
	char buff[kRecvBuffSizeAreas];

	// バッファに受信
	ret = recv( sid, buff, 5, RECV_FLAG );

	if ( ret <= 0 )	return -1;	// 受信失敗
	printf("Android LLAH %s!\n", buff);
	return ret;
}

int SendDoc( SOCKET sid, char *doc_name, struct sockaddr_in *addr, int ptc )
	// 文書名と射影変換パラメータを送信
{
	int ret, length=0, buff_cur=0;
	char buff[20]="";

	//memset( buff, 0, 24 );
	//length = strlen(doc_name);
	////printf("%d\n", length);
	//memcpy( buff, &length, 4);
	//buff_cur += 4;
	memcpy( buff, doc_name, kMaxDocNameLen );
	ret = send ( sid, buff, 20, SEND_FLAG );

	return ret;
}
			//double data_A[] = {param.a1, param.b1, param.c1, param.a2, param.b2, param.c2, param.a3, param.b3, 1.0};
			//CvMat *mat = cvCreateMat(3, 3, CV_64F);

			//for ( int i = 0; i < 3; i++ ) {
			//	for ( int j = 0; j < 3; j++ ) {
			//		cvmSet(mat, i, j, data_A[i*3+j]);
			//	}
			//}
			//CvMat *H = cvCreateMat(3, 3, CV_64F);
			//cvCopy(mat, H);
			////for ( int i = 0; i < 3; i++ ) {
			////	for ( int j = 0; j < 3; j++ ) {
			////		printf("before : %lf\n", cvmGet(mat, i, j));
			////	}
			////}
			//CvMat *tenchi = cvCreateMat(3,3,CV_64F);
			//cvTranspose(mat, tenchi);
			////for ( int i = 0; i < 3; i++ ) {
			////	for ( int j = 0; j < 3; j++ ) {
			////		printf("tenchi : %lf\n", cvmGet(tenchi, i, j));
			////	}
			////}
			//cvMatMul(tenchi, mat, mat);
			////for ( int i = 0; i < 3; i++ ) {
			////	for ( int j = 0; j < 3; j++ ) {
			////		printf("after : %lf\n", cvmGet(mat, i, j));
			////	}
			////}
			//CvMat *eigenv = cvCreateMat(3, 3, CV_64F);
			//CvMat *eigen = cvCreateMat(3, 1, CV_64F);
			//cvEigenVV(mat, eigenv, eigen, 0.0, -1, -1);

			////for ( int i = 0; i < 3; i++ ) {
			////	printf("value : %lf\n", cvmGet(eigen, i, 0));
			////	for ( int j = 0; j < 3; j++ ) {
			////		printf("vector : %lf\n", cvmGet(eigenv, j, i));
			////	}
			////}


			//CvMat *u1, *u2, *v1, *v2, *v3, *tmp1, *tmp2;
			//u1 = cvCreateMat(3,1,CV_64F);
			//u2 = cvCreateMat(3,1,CV_64F);
			//v1 = cvCreateMat(3,1,CV_64F);
			//v2 = cvCreateMat(3,1,CV_64F);
			//v3 = cvCreateMat(3,1,CV_64F);
			//for ( int i = 0; i < 3; i++ ) {
			//	cvmSet(v1, i, 0, cvmGet(eigenv, i, 0) );
			//	//printf("v1 : %lf\n", cvmGet(v1, i, 0));
			//}
			//for ( int i = 0; i < 3; i++ ) {
			//	cvmSet(v2, i, 0, cvmGet(eigenv, i, 1) );
			//	//printf("v2 : %lf\n", cvmGet(v2, i, 0));
			//}
			//for ( int i = 0; i < 3; i++ ) {
			//	cvmSet(v3, i, 0, cvmGet(eigenv, i, 2) );
			//	//printf("v3 : %lf\n", cvmGet(v3, i, 0));
			//}

			//tmp1 = cvCreateMat(3,1,CV_64F);
			//tmp2 = cvCreateMat(3,1,CV_64F);
			//double sigma2=cvmGet(eigen,1,0);
			//double sigma1=cvmGet(eigen,0,0)/sigma2, sigma3=cvmGet(eigen,2,0)/sigma2;

			//cvAddWeighted( v1, sqrt(1-sigma3*sigma3), v1, 0, 0, tmp1);
			////for ( int i = 0; i < 3; i++ ) {
			////	printf("tmp1 : %lf\n", cvmGet(tmp1, i, 0));
			////}
			//cvAddWeighted( v3, sqrt(sigma1*sigma1-1), v3, 0, 0, tmp2);
			////for ( int i = 0; i < 3; i++ ) {
			////	printf("tmp2 : %lf\n", cvmGet(tmp2, i, 0));
			////}
			//cvAdd(tmp1, tmp2, u1);
			//cvAddWeighted( u1, 1.0/sqrt(sigma1*sigma1-sigma3*sigma3), u1, 0, 0, u1);

			//cvSub(tmp1, tmp2, u2);
			//cvAddWeighted( u2, 1.0/sqrt(sigma1*sigma1-sigma3*sigma3), u2, 0, 0, u2);
			////for ( int i = 0; i < 3; i++ ) {
			////	printf("u1 : %lf\n", cvmGet(u1, i, 0));
			////}
			////for ( int i = 0; i < 3; i++ ) {
			////	printf("u2 : %lf\n", cvmGet(u2, i, 0));
			////}

			//CvMat *v2u1 = cvCreateMat(3,1,CV_64F);
			//CvMat *v2u2 = cvCreateMat(3,1,CV_64F);
			//cvCrossProduct(v2,u1,v2u1);
			//cvCrossProduct(v2,u2,v2u2);
			////for ( int i = 0; i < 3; i++ ) {
			////	printf("v2u1 : %lf\n", cvmGet(v2u1, i, 0));
			////}
			////for ( int i = 0; i < 3; i++ ) {
			////	printf("v2u2 : %lf\n", cvmGet(v2u2, i, 0));
			////}

			//CvMat *U1 = cvCreateMat(3,3,CV_64F);
			//CvMat *U2 = cvCreateMat(3,3,CV_64F);

			//for ( int i = 0; i < 3; i++ ) {
			//	cvmSet(U1,i,0,cvmGet(v2, i, 0));
			//	cvmSet(U1,i,1,cvmGet(u1, i, 0));
			//	cvmSet(U1,i,2,cvmGet(v2u1, i, 0));
			//}
			//for ( int i = 0; i < 3; i++ ) {
			//	cvmSet(U2,i,0,cvmGet(v2, i, 0));
			//	cvmSet(U2,i,1,cvmGet(u2, i, 0));
			//	cvmSet(U2,i,2,cvmGet(v2u2, i, 0));
			//}
			////for ( int i = 0; i < 3; i++ ) {
			////	for ( int j = 0; j < 3; j++ ) {
			////		printf("U1 : %lf\n", cvmGet(U1, i, j));
			////	}
			////}
			////for ( int i = 0; i < 3; i++ ) {
			////	for ( int j = 0; j < 3; j++ ) {
			////		printf("U2 : %lf\n", cvmGet(U2, i, j));
			////	}
			////}

			//CvMat *W1 = cvCreateMat(3,3,CV_64F);
			//CvMat *W2 = cvCreateMat(3,3,CV_64F);
			//cvAddWeighted( H, 1.0/sigma2, H, 0, 0, H);
			//CvMat *Hv2 = cvCreateMat(3,1,CV_64F);
			//CvMat *Hu1 = cvCreateMat(3,1,CV_64F);
			//CvMat *Hu2 = cvCreateMat(3,1,CV_64F);
			//CvMat *Hv2Hu1 = cvCreateMat(3,1,CV_64F);
			//CvMat *Hv2Hu2 = cvCreateMat(3,1,CV_64F);

			//cvMatMul(H, v2, Hv2);
			//cvMatMul(H, u1, Hu1);
			//cvMatMul(H, u2, Hu2);
			//cvCrossProduct(Hv2,Hu1,Hv2Hu1);
			//cvCrossProduct(Hv2,Hu2,Hv2Hu2);

			//for ( int i = 0; i < 3; i++ ) {
			//	cvmSet(W1,i,0,cvmGet(Hv2, i, 0));
			//	cvmSet(W1,i,1,cvmGet(Hu1, i, 0));
			//	cvmSet(W1,i,2,cvmGet(Hv2Hu1, i, 0));
			//}
			//for ( int i = 0; i < 3; i++ ) {
			//	cvmSet(W2,i,0,cvmGet(Hv2, i, 0));
			//	cvmSet(W2,i,1,cvmGet(Hu2, i, 0));
			//	cvmSet(W2,i,2,cvmGet(Hv2Hu2, i, 0));
			//}

			//CvMat *U1tenchi = cvCreateMat(3,3,CV_64F);
			//CvMat *R1 = cvCreateMat(3,3,CV_64F);
			//CvMat *T1tmp = cvCreateMat(3,3,CV_64F);
			//CvMat *T1 = cvCreateMat(3,1,CV_64F);

			//cvTranspose(U1, U1tenchi);
			//// R
			//cvMatMul(W1, U1tenchi, R1);
			//// T
			//cvSub(H, R1, T1tmp);
			//cvMatMul(T1tmp, v2u1, T1);

			////puts("H1 :");
			////for ( int i = 0; i < 3; i++ ) {
			////	for ( int j = 0; j < 3; j++ ) {
			////		printf("%lf\t", cvmGet(H, i, j));
			////	}
			////	puts("");
			////}
			////puts("");

			//puts("R1 :\t\t\t\t\t\tT1 :");
			//for ( int i = 0; i < 3; i++ ) {
			//	for ( int j = 0; j < 3; j++ ) {
			//		printf("%lf\t", cvmGet(U1, i, j));
			//	}
			//	printf("%lf\n", cvmGet(T1, i, 0));
			//}
			//puts("");
			//
			//CvMat *N1tenchi = cvCreateMat(1,3,CV_64F);
			//CvMat *_H = cvCreateMat(3,3,CV_64F);

			//cvTranspose(v2u1, N1tenchi);
			//cvMatMul(T1, N1tenchi, _H);
			//cvAdd(R1, _H, _H);

			////puts("_H :");
			////for ( int i = 0; i < 3; i++ ) {
			////	for ( int j = 0; j < 3; j++ ) {
			////		printf("%lf\t", cvmGet(_H, i, j));
			////	}
			////	puts("");
			////}
			////puts("");

			//CvMat *U2tenchi = cvCreateMat(3,3,CV_64F);
			//CvMat *R2 = cvCreateMat(3,3,CV_64F);
			//CvMat *T2tmp = cvCreateMat(3,3,CV_64F);
			//CvMat *T2 = cvCreateMat(3,1,CV_64F);
			//cvTranspose(U2, U2tenchi);
			//// R
			//cvMatMul(W2, U2tenchi, R2);
			//// T
			//cvSub(H, R2, T2tmp);
			//cvMatMul(T1tmp, v2u2, T2);

			////puts("R2 :\t\t\t\t\t\tT2 :");
			////for ( int i = 0; i < 3; i++ ) {
			////	for ( int j = 0; j < 3; j++ ) {
			////		printf("%lf\t", cvmGet(U2, i, j));
			////	}
			////	printf("%lf\n", cvmGet(T2, i, 0));
			////}
			////puts("");
