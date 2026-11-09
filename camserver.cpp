#include "def_general.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>

#ifdef WIN32
#ifdef _DEBUG
#pragma comment( lib, "opencv_imgproc4d.lib" )
#pragma comment( lib, "opencv_core4d.lib" )
#pragma comment( lib, "opencv_highgui4d.lib" )
#pragma comment( lib, "opencv_calib3d4d.lib" )
#else
#pragma comment( lib, "opencv_imgproc4.lib" )
#pragma comment( lib, "opencv_core4.lib" )
#pragma comment( lib, "opencv_highgui4.lib" )
#pragma comment( lib, "opencv_calib3d4.lib" )
#endif
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
	cv::Point *ps=NULL, **reg_pss=NULL;
	cv::Size img_size, *reg_sizes=NULL, res_size;
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
	//if ( ( sid1 = InitWinSockSvTCP( eTCPPort, "192.168.42.129") ) < 0 )	exit(1);	// socketの初期化
	if ( ( sid1 = InitWinSockSvTCP( 12345 ) ) < 0 )	exit(1);	// socketの初期化
	//if ( ( sid1 = InitWinSockSvTCP( eTCPPort, eServerName) ) < 0 )	exit(1);	// socketの初期化
	if ( ( sid2 = AcceptWinSockSvTCP( sid1 ) ) < 0 )	exit(1);	// accept
#else
	if ( ( sid1 = InitSockSvTCP( eTCPPort, eServerName ) ) < 0 )	exit(1);	// socketの初期化
	if ( ( sid2 = AcceptSockSvTCP( sid1 ) ) < 0 )	exit(1);	// accept
#endif
	cout << "connected " << endl;

	zero_param.a1 = 0.0; zero_param.a2 = 0.0; zero_param.a3 = 0.0; zero_param.b1 = 0.0; zero_param.b2 = 0.0; zero_param.b3 = 0.0; zero_param.c1 = 0.0; zero_param.c2 = 0.0;
	param = zero_param;
	res_size = reg_sizes[0];
	strcpy( doc_name, doc_reject );
	pcornum=0;
	res=0;

	//ret = SendDoc( sid2, "test", &addr, eProtocol );
	//ret = SendResultParam( sid2, doc_name, kMaxDocNameLen, &param, &res_size, &addr, eProtocol );
	//ret = SendResultParam3D( sid2, doc_name, kMaxDocNameLen, &param, tranceParam1, tranceParam2, &res_size, &addr, eProtocol );
	SendResultName( sid2, "reject" );

	for ( ; ; ) {
		num = RecvPointsAreas( sid2, &ps, &areas, &img_size );	// 特徴点データを受信
		if ( num < 0 )	break;	// 通信失敗

		if ( num >= kMinPoints ) {	// 特徴点数が最小値以上
			MakeNearsFromCentresDiv( ps, num, &img_size, kDivX, kDivY, eGroup1Num );	// 近傍点計算（分割版）
			res = RetrieveCor( ps, areas, num, score, pcor, &pcornum, &disc, reg_nums, hash );
			// 最小対応点数以上なら、射影変換パラメータを計算
			if ( pcornum >= kMinPointsToCalcParam ) {
				CalcProjParamTop( ps, reg_pss[res], pcor, pcornum, &param, PROJ_REVERSE, PARAM_RANSAC );	// 登録画像に上書き
				CalcRotationAndTranslation(ps, reg_pss[res], pcor, pcornum, tranceParam1);
			}
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

		//ret = SendResultParam( sid2, doc_name, kMaxDocNameLen, &param, &res_size, &addr, eProtocol );
		//ret = SendResultParam3D( sid2, doc_name, kMaxDocNameLen, &param, tranceParam1, tranceParam2, &res_size, &addr, eProtocol );
		SendResultName( sid2, doc_name );


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

void CalcRotationAndTranslation( cv::Point *ps, cv::Point *reg_ps, int corres[][2], int cor_num, float *tranceParam )
{
	double intri[3][3]	= {{5.77397522e+002, 0, 320}, {0, 5.76541565e+002, 240}, {0, 0, 1}};
	double dist_data[4]		= {3.29653658e-002, -2.11595669e-001, -5.16368134e-004, -7.00226147e-003};

	std::vector<cv::Point3f> objectPoints(cor_num);
	std::vector<cv::Point2f> imagePoints(cor_num);

	for ( int i = 0; i < cor_num; i++ ) {
		objectPoints[i].x = (float)reg_ps[corres[i][1]].x;
		objectPoints[i].y = (float)reg_ps[corres[i][1]].y;
		objectPoints[i].z = 0.0f;
	}

	for ( int i = 0; i < cor_num; i++ ) {
		imagePoints[i].x = (float)ps[corres[i][0]].x;
		imagePoints[i].y = (float)ps[corres[i][0]].y;
	}

	cv::Mat cameraMatrix(3, 3, CV_64F, intri);
	cv::Mat distCoeffs(1, 4, CV_64F, dist_data);
	cv::Mat rvec, tvec;

	cv::solvePnP(objectPoints, imagePoints, cameraMatrix, distCoeffs, rvec, tvec);

	double _r1[3];
	_r1[0] = rvec.at<double>(0, 0);
	_r1[1] = rvec.at<double>(1, 0);
	_r1[2] = rvec.at<double>(2, 0);

	//for ( int i = 0; i < 3; i++ )	cout << _r1[i] << " ";
	//cout << endl;

	double trans[3];
	trans[0] = tvec.at<double>(0, 0);
	trans[1] = tvec.at<double>(1, 0);
	trans[2] = tvec.at<double>(2, 0);

	cv::Mat r1Mat(3, 1, CV_64F, _r1);
	cv::Mat _Homograph(3, 3, CV_64F);

	//3x3に変換
	cv::Rodrigues(r1Mat, _Homograph);

	//for ( int i = 0; i < 3; i++ ) {
	//	for ( int j = 0; j < 3; j++ ) {
	//		printf("%lf\t", _Homograph.at<double>(i, j));
	//	}
	//	cout << trans[i] << endl;
	//}
	//cout << endl;

	double sinthe = -_Homograph.at<double>(0, 2);
	double costhe = sqrt(1.0-sinthe*sinthe);
	double sinpsy = _Homograph.at<double>(1, 2) / costhe;
	double sinfai = _Homograph.at<double>(0, 1) / costhe;
	double cospsy = _Homograph.at<double>(2, 2) / costhe;
	double cosfai = _Homograph.at<double>(0, 0) / costhe;

	sinfai = -sinfai;
	sinthe = -sinthe;

	_Homograph.at<double>(0, 0) = cosfai * costhe;
	_Homograph.at<double>(0, 1) = sinfai * costhe;
	_Homograph.at<double>(0, 2) = -sinthe;
	_Homograph.at<double>(1, 0) = cosfai * sinthe * sinpsy - sinfai * cospsy;
	_Homograph.at<double>(1, 1) = sinfai * sinthe * sinpsy + cosfai * cospsy;
	_Homograph.at<double>(1, 2) = costhe * sinpsy;
	_Homograph.at<double>(2, 0) = cosfai * sinthe * cospsy + sinfai * sinpsy;
	_Homograph.at<double>(2, 1) = sinfai * sinthe * cospsy - cosfai * sinpsy;
	_Homograph.at<double>(2, 2) = costhe * cospsy;
	
	for ( int i = 0; i < 3; i++ ) {
		tranceParam[0+i] = (float)_Homograph.at<double>(i, 0);
		tranceParam[4+i] = (float)_Homograph.at<double>(i, 1);
		tranceParam[8+i] = (float)_Homograph.at<double>(i, 2);
		tranceParam[12+i] = (float)trans[i];
	}
	tranceParam[3] = 0.0;
	tranceParam[7] = 0.0;
	tranceParam[11] = 0.0;
	tranceParam[15] = 1.0;
}

double normalize ( Vec3f&  vec )
{
	float a = vec[0] * vec[0];
	float b = vec[1] * vec[1];
	float c = vec[2] * vec[2];

	float norm = sqrt ( a + b + c );

	vec[0] /= norm;
	vec[1] /= norm;
	vec[2] /= norm;

	return norm;
}

void ExtractRAndTFromP2(strProjParam param, float *tranceParam)
{
	double data_H[3][3] = {{param.a1, param.b1, param.c1}, {param.a2, param.b2, param.c2}, {param.a3, param.b3, 1.0}};	// ホモグラフィ行列

	// A[0][2]とA[1][2]を0.0に変更してみてください．
	double data_A[3][3] = {{569.65435791, 0.0, 0.0}, {0.0, 568.74615479, 0.0}, {0.0, 0.0, 1.0}};		// カメラ内部パラメータ
	//double data_A[3][3] = {{2.51902783e+003, 0.0, 1.29164893e+003}, {0.0, 2.51729639e+003, 9.62208679e+002}, {0.0, 0.0, 1.0}};		// カメラ内部パラメータ
	//double data_A[3][3] = {{569.65435791, 0.0, 321.98031616}, {0.0, 568.74615479, 237.28897095}, {0.0, 0.0, 1.0}};		// カメラ内部パラメータ
	
	data_A[0][2] = 0.0;
	data_A[1][2] = 0.0;


	Mat HL(3, 3, CV_64F, data_H);	// ホモグラフィ行列
	Mat A(3, 3, CV_64F, data_A);	// カメラ内部パラメータ

	Mat RT = A.inv() * HL;

	Vec3f rVec1 = Vec3f( RT.at< double >(0,0), RT.at< double >(1,0), RT.at< double >(2,0) );
	Vec3f rVec2 = Vec3f( RT.at< double >(0,1), RT.at< double >(1,1), RT.at< double >(2,1) );
	Vec3f tVec  = Vec3f( RT.at< double >(0,2), RT.at< double >(1,2), RT.at< double >(2,2) );

	Vec3f rVec3 = rVec1.cross ( rVec2 );

	double norm = normalize ( rVec1 );
	normalize ( rVec2 );
	normalize ( rVec3 );
	for ( int i = 0; i < 3; i++ ) {
		tVec[i] /= norm;
	}

	//for ( int i = 0; i < 3; i++ ) {
	//	cout << rVec1[i] << "\t";
	//	cout << rVec2[i] << "\t";
	//	cout << rVec3[i] << "\t";
	//	cout << tVec[i] << endl;
	//}
	//puts("");

	tranceParam[3] = 0.0;
	tranceParam[7] = 0.0;
	tranceParam[11] = 0.0;
	tranceParam[15] = 1.0;

	double sinthe = -rVec1[2];
	double costhe = sqrt(1.0-sinthe*sinthe);
	double sinpsy = rVec2[2] / costhe;
	double sinfai = rVec1[1] / costhe;
	double cospsy = rVec3[2] / costhe;
	double cosfai = rVec1[0] / costhe;

	//double eulerAngleX = acos(cospsy) * (sinpsy >= 0 ? 1 : -1) * (180.0 / CV_PI);
	//double eulerAngleY = acos(costhe) * (sinthe >= 0 ? 1 : -1) * (180.0 / CV_PI);
	//double eulerAngleZ = acos(cosfai) * (sinfai >= 0 ? 1 : -1) * (180.0 / CV_PI);

	//cout << "Angle X : " << eulerAngleX;
	//cout << " Angle Y : " << eulerAngleY;
	//cout << " Angle Z : " << eulerAngleZ << endl;


	//sinpsy = -sinpsy;
	//if ( sinthe < 0 )	sinthe = -sinthe;
	//sinfai = -sinfai;


	rVec1[0] = cosfai * costhe;
	rVec2[0] = sinfai * costhe;
	rVec3[0] = -sinthe;
	rVec1[1] = cosfai * sinthe * sinpsy - sinfai * cospsy;
	rVec2[1] = sinfai * sinthe * sinpsy + cosfai * cospsy;
	rVec3[1] = costhe * sinpsy;
	rVec1[2] = cosfai * sinthe * cospsy + sinfai * sinpsy;
	rVec2[2] = sinfai * sinthe * cospsy - cosfai * sinpsy;
	rVec3[2] = costhe * cospsy;

	for ( int i = 0; i < 3; i++ ) {
		tranceParam[0+i] = rVec1[i];
		tranceParam[4+i] = rVec2[i];
		tranceParam[8+i] = rVec3[i];
		tranceParam[12+i] = tVec[i];
		//cout << rVec1[i] << "\t";
		//cout << rVec2[i] << "\t";
		//cout << rVec3[i] << "\t";
		//cout << tVec[i] << endl;
	}
	//puts("");
}



void ExtractRAndTFromP(strProjParam param, float *tranceParam1, float *tranceParam2)
// 射影変換パラメータから回転行列Rと併進行列Tを抽出する
{
	double data_A[] = {param.a1, param.b1, param.c1, param.a2, param.b2, param.c2, param.a3, param.b3, 1.0};
	// ホモグラフィ行列
	cv::Mat Hl(3, 3, CV_64F, data_A);
	cv::Mat Hl_tenchi, HlHl;
	cv::Mat eigenVec(3, 3, CV_64F), eigenVal(3, 1, CV_64F);
	cv::Mat v1(3, 1, CV_64F), v2(3, 1, CV_64F), v3(3, 1, CV_64F);
	cv::Mat u1(3, 1, CV_64F), u2(3, 1, CV_64F);
	cv::Mat v2u1(3, 1, CV_64F), v2u2(3, 1, CV_64F);
	cv::Mat U1(3, 3, CV_64F), U2(3, 3, CV_64F);
	cv::Mat H(3, 3, CV_64F);
	cv::Mat Hv2(3, 1, CV_64F), Hu1(3, 1, CV_64F), Hu2(3, 1, CV_64F);
	cv::Mat Hv2Hu1(3, 1, CV_64F), Hv2Hu2(3, 1, CV_64F);
	cv::Mat W1(3, 3, CV_64F), W2(3, 3, CV_64F);
	cv::Mat U1tenchi, U2tenchi;
	cv::Mat R1(3, 3, CV_64F), R2(3, 3, CV_64F);
	cv::Mat T1tmp(3, 3, CV_64F), T2tmp(3, 3, CV_64F);
	cv::Mat T1(3, 1, CV_64F), T2(3, 1, CV_64F);
	double sigma1=0.0, sigma2=0.0, sigma3=0.0;

	// Hl is already initialized from data_A

	// ホモグラフィ行列の転置行列
	Hl_tenchi = Hl.t();
	HlHl = Hl_tenchi * Hl;
	// HlHlの固有値分解
	cv::eigen(HlHl, eigenVal, eigenVec);

	// 行列から固有ベクトルの抽出
	for ( int i = 0; i < 3; i++ ) {
		v1.at<double>(i, 0) = eigenVec.at<double>(i, 0);
		v2.at<double>(i, 0) = eigenVec.at<double>(i, 1);
		v3.at<double>(i, 0) = eigenVec.at<double>(i, 2);
	}

	//for ( int i = 0; i < 3; i++ ) {
	//	for ( int j = 0; j < 3; j++ ) {
	//		printf("%lf\t", eigenVec.at<double>(i, j));
	//	}
	//	puts("");
	//}
	//puts("");
	puts("eigen");
	for ( int i = 0; i < 3; i++ ) {
		printf("%lf\t", eigenVal.at<double>(i, 0));
	}
	puts("");
	puts("v1");
	for ( int i = 0; i < 3; i++ ) {
		printf("%lf\t", v1.at<double>(i, 0));
	}
	puts("");
	puts("v2");
	for ( int i = 0; i < 3; i++ ) {
		printf("%lf\t", v2.at<double>(i, 0));
	}
	puts("");
	puts("v3");
	for ( int i = 0; i < 3; i++ ) {
		printf("%lf\t", v3.at<double>(i, 0));
	}
	puts("");
	puts("");
	sigma2=eigenVal.at<double>(1,0);
	sigma1=eigenVal.at<double>(0,0)/sigma2;
	sigma3=eigenVal.at<double>(2,0)/sigma2;

	cv::Mat tmp1(3, 1, CV_64F), tmp2(3, 1, CV_64F);

	// u1
	cv::addWeighted( v1, sqrt(1-sigma3), v1, 0, 0, tmp1);
	cv::addWeighted( v3, sqrt(sigma1-1), v3, 0, 0, tmp2);
	u1 = tmp1 + tmp2;
	cv::addWeighted( u1, 1.0/sqrt(sigma1-sigma3), u1, 0, 0, u1);

	// u2
	u2 = tmp1 - tmp2;
	cv::addWeighted( u2, 1.0/sqrt(sigma1-sigma3), u2, 0, 0, u2);

	// cross product
	v2u1 = v2.cross(u1);
	v2u2 = v2.cross(u2);
	
	for ( int i = 0; i < 3; i++ ) {
		U1.at<double>(i,0) = v2.at<double>(i, 0);
		U1.at<double>(i,1) = u1.at<double>(i, 0);
		U1.at<double>(i,2) = v2u1.at<double>(i, 0);

		U2.at<double>(i,0) = v2.at<double>(i, 0);
		U2.at<double>(i,1) = u2.at<double>(i, 0);
		U2.at<double>(i,2) = v2u2.at<double>(i, 0);
	}
	double dv2 = 0.0;
	for ( int i = 0; i < 3; i++ ) {
		dv2 += v2.at<double>(i, 0) * v2.at<double>(i, 0);
	}
	printf("v2 : %lf\n", sqrt(dv2));
	puts("v2 : ");
	for ( int i = 0; i < 3; i++ ) {
		printf("%lf\t", v2.at<double>(i, 0));
	}
	puts("");
	double du = cv::determinant(U1);
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
	cv::addWeighted( Hl, 1.0/sqrt(sigma2), Hl, 0, 0, H);

	cv::Mat H_tenchi, HH;
	H_tenchi = H.t();
	HH = H_tenchi * H;
	cv::eigen(HH, eigenVal, eigenVec);

	// HlHlの固有値分解
	printf("v2 : %lf\n", eigenVal.at<double>(1,0));

	puts("H : ");
	for ( int i = 0; i < 3; i++ ) {
		for ( int j = 0; j < 3; j++ ) {
			printf("%lf\t", H.at<double>(i, j));
		}
		puts("");
	}
	puts("");
	
	
	double dh = cv::determinant(H);
	printf("H : %lf\n", dh);

	Hv2 = H * v2;
	Hu1 = H * u1;
	Hu2 = H * u2;
	Hv2Hu1 = Hv2.cross(Hu1);
	Hv2Hu2 = Hv2.cross(Hu2);

	puts("Hv2");
	for ( int i = 0; i < 3; i++ ) {
			printf("%lf\t", Hv2.at<double>(i, 0));
	}
	puts("");

	puts("Hu1");
	for ( int i = 0; i < 3; i++ ) {
			printf("%lf\t", Hu1.at<double>(i, 0));
	}
	puts("");

	puts("Hv2Hu1");
	for ( int i = 0; i < 3; i++ ) {
			printf("%lf\t", Hv2Hu1.at<double>(i, 0));
	}
	puts("");

	double dHv2 = 0.0;
	for ( int i = 0; i < 3; i++ ) {
		dHv2 += Hv2.at<double>(i, 0) * Hv2.at<double>(i, 0);
	}
	printf("Hv2 : %lf\n", sqrt(dHv2));

	double dHu1 = 0.0;
	for ( int i = 0; i < 3; i++ ) {
		dHu1 += Hu1.at<double>(i, 0) * Hu1.at<double>(i, 0);
	}
	printf("Hu1 : %lf\n", sqrt(dHu1));

	double dHu2 = 0.0;
	for ( int i = 0; i < 3; i++ ) {
		dHu2 += Hu2.at<double>(i, 0) * Hu2.at<double>(i, 0);
	}
	printf("Hu2 : %lf\n", sqrt(dHu2));

	double dHv2Hu1 = 0.0;
	for ( int i = 0; i < 3; i++ ) {
		dHv2Hu1 += Hv2Hu1.at<double>(i, 0) * Hv2Hu1.at<double>(i, 0);
	}
	printf("Hv2Hu1 : %lf\n", sqrt(dHv2Hu1));

	double dHv2Hu2 = 0.0;
	for ( int i = 0; i < 3; i++ ) {
		dHv2Hu2 += Hv2Hu2.at<double>(i, 0) * Hv2Hu2.at<double>(i, 0);
	}
	printf("Hv2Hu2 : %lf\n", sqrt(dHv2Hu2));

	for ( int i = 0; i < 3; i++ ) {
		W1.at<double>(i,0) = Hv2.at<double>(i, 0);
		W1.at<double>(i,1) = Hu1.at<double>(i, 0);
		W1.at<double>(i,2) = Hv2Hu1.at<double>(i, 0);

		W2.at<double>(i,0) = Hv2.at<double>(i, 0);
		W2.at<double>(i,1) = Hu2.at<double>(i, 0);
		W2.at<double>(i,2) = Hv2Hu2.at<double>(i, 0);
	}
	//puts("W1 : ");

	for ( int i = 0; i < 3; i++ ) {
		for ( int j = 0; j < 3; j++ ) {
			printf("%lf\t", W1.at<double>(i, j));
		}
		puts("");
	}
	puts("");
	double dw = cv::determinant(W1);
	printf("W1 : %lf\n", dw);
	// 真のホモグラフィ行列（Hlの正規化？）
	cv::addWeighted( W1, 1.0/pow(dw, 1.0/3.0), W1, 0, 0, W1);
	dw = cv::determinant(W1);
	printf("W1 : %lf\n", dw);

	for ( int i = 0; i < 3; i++ ) {
		for ( int j = 0; j < 3; j++ ) {
			printf("%lf\t", W1.at<double>(i, j));
		}
		puts("");
	}
	puts("");
	double dw2 = cv::determinant(W2);
	printf("W2 : %lf\n", dw2);
	// 真のホモグラフィ行列（Hlの正規化？）
	cv::addWeighted( W2, 1.0/dw2, W2, 0, 0, W2);
	dw2 = cv::determinant(W2);
	printf("W2 : %lf\n", dw2);
	for ( int i = 0; i < 3; i++ ) {
		for ( int j = 0; j < 3; j++ ) {
			printf("%lf\t", W1.at<double>(i, j));
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

	U1tenchi = U1.t();
	// R1 = W1U1
	R1 = W1 * U1tenchi;
	// T1 = (H-R1)N1
	T1tmp = H - R1;
	T1 = T1tmp * v2u1;

	// R2,T2の抽出（R,Tは2つできる）
	U2tenchi = U2.t();
	// R2 = W2U2
	R2 = W2 * U2tenchi;
	// T2 = (H-R2)N2
	T2tmp = H - R2;
	T2 = T1tmp * v2u2;

	// 配列に格納
	for ( int i = 0; i < 3; i++ ) {
		tranceParam1[0+4*i] = (float)R1.at<double>(0,i);
		tranceParam1[1+4*i] = (float)R1.at<double>(1,i);
		tranceParam1[2+4*i] = (float)R1.at<double>(2,i);
	}
	for ( int i = 0; i < 3; i++ ) {
		tranceParam1[12+i] = (float)T1.at<double>(i,0);
	}
	tranceParam1[3] = 0.0;
	tranceParam1[7] = 0.0;
	tranceParam1[11] = 0.0;
	tranceParam1[15] = 1.0;

	// 配列に格納
	for ( int i = 0; i < 3; i++ ) {
		tranceParam2[0+4*i] = (float)R2.at<double>(0,i);
		tranceParam2[1+4*i] = (float)R2.at<double>(1,i);
		tranceParam2[2+4*i] = (float)R2.at<double>(2,i);
	}
	for ( int i = 0; i < 3; i++ ) {
		tranceParam2[12+i] = (float)T2.at<double>(i,0);
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
	cv::Mat N1tenchi = v2u1.t();
	cv::Mat _H = T1 * N1tenchi;
	_H = R1 + _H;

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

void SendResultName( SOCKET sock, char *result_name )
{
	send ( sock, result_name, kFileNameLen, SEND_FLAG );
}

int SendResultParam( SOCKET sid, char *doc_name, int len, strProjParam *param, cv::Size *img_size, struct sockaddr_in *addr, int ptc )
	// 文書名と射影変換パラメータを送信
{
	int ret;
	char buff[kSendBuffSize];

	memcpy( buff, doc_name, kMaxDocNameLen );
	memcpy( buff + kMaxDocNameLen, param, sizeof(strProjParam) );
	memcpy( buff + kMaxDocNameLen + sizeof(strProjParam), img_size, sizeof(cv::Size) );

	ret = send ( sid, buff, kSendBuffSize, SEND_FLAG );

	return ret;
}

int SendResultParam3D( SOCKET sid, char *doc_name, int len, strProjParam *param, float *tranceParam1, float *tranceParam2, cv::Size *img_size, struct sockaddr_in *addr, int ptc )
	// 文書名と射影変換パラメータを送信
{
	int ret;
	char buff[kSendBuffSize3D];

	memcpy( buff, doc_name, kMaxDocNameLen );
	memcpy( buff + kMaxDocNameLen, param, sizeof(strProjParam) );
	memcpy( buff + kMaxDocNameLen + sizeof(strProjParam), img_size, sizeof(cv::Size) );
	memcpy( buff + kMaxDocNameLen + sizeof(strProjParam) + sizeof(cv::Size), tranceParam1, sizeof(float)*16 );

	ret = send ( sid, buff, kSendBuffSize3D, SEND_FLAG );

	return ret;
}

int SendResultCor( SOCKET sid, char *doc_name, int len, strProjParam *param, cv::Size size, int pcor[][2], int pcornum, cv::Point *ps, cv::Point *psall, struct sockaddr_in *addr, int ptc )
	// 文書名と対応点を送信
{
	int i, buff_cur=0, ret;
	char buff[kSendBuffSizeCor];
	cv::Point corps[kMaxPointNum][2];

	puts(doc_name);

	memcpy( buff, doc_name, kMaxDocNameLen );
	buff_cur += kMaxDocNameLen;
	memcpy( buff + buff_cur, param, sizeof(strProjParam) );
	buff_cur += sizeof(strProjParam);
	memcpy( buff + buff_cur, &size, sizeof(cv::Size) );
	buff_cur += sizeof(cv::Size);
	memcpy( buff + buff_cur, &pcornum, sizeof(int) );
	buff_cur += sizeof(int);
	for ( i = 0; i < pcornum; i++ ) {
		memcpy( buff + buff_cur, &(ps[pcor[i][0]]), sizeof(cv::Point) );
		buff_cur += 8;
	}
	for ( i = 0; i < pcornum; i++ ) {
		memcpy( buff + buff_cur, &(psall[pcor[i][1]]), sizeof(cv::Point) );
		buff_cur += 8;
	}

	if ( ptc == kTCP ) 
		ret = send( sid, buff, kSendBuffSizeCor, SEND_FLAG );
	else
		ret = sendto( sid, buff, kSendBuffSizeCor, SEND_FLAG, (struct sockaddr *)addr, sizeof(*addr) );

	return ret;
}

int RecvPointsAreas( SOCKET sid, cv::Point **ps0, double **areas0, cv::Size *size )
	// 点データを受信
{
	int i, num = 0, ret=0, buff_cur, total=0;
	char buff[kRecvBuffSizeAreas];
	cv::Point *ps;
	double *areas;
	unsigned short areas_us[kMaxPointNum];

	// receive to buffer
	while (total < 25012) {
		ret = recv( sid, &buff[total], 25012-total, RECV_FLAG );
		total += ret;
	}
	if ( ret <= 0 )	return -1;
	// バッファから画像サイズを得る
	for ( i = 0, buff_cur = 0; i < (int)sizeof(cv::Size); i++, buff_cur++ ) {
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
		ps = (cv::Point *)calloc( num, sizeof(cv::Point) );
		areas = (double *)calloc( num, sizeof(double) );
	}
	else {
		ps = NULL;
		areas = NULL;
	}
	*ps0 = ps;
	*areas0 = areas;
	// バッファから特徴点データを得る
	for ( i = 0; i < (int)(sizeof(cv::Point) * num); i++, buff_cur++ ) {
		((char *)ps)[i] = buff[buff_cur];
	}
	//for ( i = 0; i < num; i++ )	printf("Point: %d, %d\n", ps[i].x, ps[i].y);

	// バッファから面積データを得る
	buff_cur = sizeof(cv::Size) + sizeof(int) + sizeof(cv::Point) * kMaxPointNum;
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




















	//double sinpsy = -rVec3[1];
	//double cospsy = sqrt(1.0-sinpsy*sinpsy);

	//double sinthe = rVec3[0] / cospsy;
	//double costhe = rVec3[2] / cospsy;
	//double sinfai = rVec1[1] / cospsy;
	//double cosfai = rVec2[1] / cospsy;


	//cout << "sinthe : " << sinthe << endl;
	//cout << "costhe : " << costhe << endl;
	//cout << "sinfai : " << sinfai << endl;
	//cout << "cosfai : " << cosfai << endl;
	//cout << "sinpsy : " << sinpsy << endl;
	//cout << "cospsy : " << cospsy << endl;
	//if ( sinpsy < 0 )	sinpsy = -sinpsy;
	//if ( sinthe < 0 )	sinthe = -sinthe;
	//if ( sinfai < 0 )	sinfai = -sinfai;


	//double tmp = sinpsy;
	//sinpsy = cospsy;
	//cospsy = -tmp;

	//sinthe = 1/sqrt(2.0);
	//costhe = 1/sqrt(2.0);
	//sinpsy = 1/sqrt(2.0);
	//cospsy = 1/sqrt(2.0);
	//sinfai = -sinfai;

	//rVec1[0] = costhe * cosfai + sinthe * sinpsy * sinfai;
	//rVec1[1] = cospsy * sinfai;
	//rVec1[2] = -sinthe * cosfai + costhe * sinpsy * sinfai;

	//rVec2[0] = -costhe * sinfai + sinthe * sinpsy * cosfai;
	//rVec2[1] = cospsy * cosfai;
	//rVec2[2] = sinthe * sinfai + costhe * sinpsy * cosfai;

	//rVec3[0] = sinthe * cospsy;
	//rVec3[1] = -sinpsy;
	//rVec3[2] = costhe * cospsy;
