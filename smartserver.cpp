#include "def_general.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// ネットワーク関連
#ifdef	WIN32
#include <winsock2.h>
#include "ws_cl.h"
#else
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "sock_sv.h"
#endif

//// マルチスレッド
//#include <pthread.h>    /* pthread_create */

// OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#ifdef _DEBUG
#pragma comment( lib, "opencv_imgproc231d.lib" )
#pragma comment( lib, "opencv_core231d.lib" )
#pragma comment( lib, "opencv_highgui231d.lib" )
#else
#pragma comment( lib, "opencv_imgproc231.lib" )
#pragma comment( lib, "opencv_core231.lib" )
#pragma comment( lib, "opencv_highgui231.lib" )
#endif
using namespace cv;

#include "extern.h"
#include "procimg.h"
#include "nears.h"
#include "load.h"
#include "gencomb.h"
#include "retrieve.h"
#include "fpath.h"

#define	SEND_FLAG	0
#ifdef	WIN32
#define	RECV_FLAG	0
#else
#define	RECV_FLAG	MSG_WAITALL
#endif

IplImage *RecvCapImg( SOCKET sock )
	// 点データを受信
{
	char size[32];
	char *buff;
	unsigned char **data;
	int buff_size=0, total=0, ret=0, width=0, height=0, buff_cur=0;
	FILE *fp;

	// 撮影画像のサイズを受信
	while ( total < 12 ) {
		ret = recv ( sock, &size[total], 12-total, RECV_FLAG );
		total += ret;
	}
	memcpy( &width, &size[0], sizeof(int) );
	memcpy( &height, &size[4], sizeof(int) );
	memcpy( &buff_size, &size[8], sizeof(int) );

	if ( width < 0 )	return NULL;

	//printf("Width: %d, Height: %d\n", width, height);
	//printf("Data Size: %d\n", buff_size);

	buff = (char *)malloc(buff_size);

	// 撮影画像を受信
	total=0;
	while ( total < buff_size ) {
		//printf("%d\n", total);
		ret = recv ( sock, &buff[total], buff_size-total, RECV_FLAG );
		total += ret;
	}
	puts("Recieved");

	// Matへの変換
	Mat mat( Size(width, height), CV_8UC1);
	for ( int i=0; i<height; i++ ) {
		for ( int j=0; j<width; j++ ) {
			mat.at<unsigned char>(i,j) = (unsigned char)buff[i*width+j];
		}
	}

	free(buff);

	IplImage img(mat);
	IplImage *cap_img = cvCloneImage(&img);

	//cvShowImage( "Capture", cap_img );
	//cvWaitKey( 0 );

	return cap_img;
}

void SendResult( SOCKET sock, char *result_name )
{
	send ( sock, result_name, kFileNameLen, SEND_FLAG );
}

int RetrieveDocImg( SOCKET sock, CvPoint **reg_pss, double **reg_areass, int *reg_nums, CvSize *reg_sizes, char **dbcors, strDisc disc, strHash **hash )
// スマフォから画像を受信し，検索，結果を送信
{
	IplImage *img, *connect;
	CvPoint *ps=NULL;
	CvSize size;
	int num;
	double *areas=NULL;

	int res=0, ret_time=0;
	char doc_name[kFileNameLen]={0};

	// 撮影画像の受信
	img = RecvCapImg( sock );
	if ( img == NULL )	return -1;

	// 得票テーブル確保
	int *score = (int *)calloc(eDbDocs, sizeof(int));
	// 結合画像生成
	connect = GetConnectedImageForSmartphone( img );
	if ( connect == NULL )	exit(-1);
	// 特徴点抽出・連結成分の面積計算
	num = MakeFeaturePoint( connect, &ps, &areas, &size );
	if ( num >= 20 ) {	// 特徴点数の閾値
		// 近傍構造の解析
		MakeNearsFromCentres( ps, num );
		// 検索
		res = Retrieve( ps, areas, num, score, &disc, reg_nums, &ret_time, hash );
		if ( score[res] > 10 ) {	// トップの得票数が閾値以下（リジェクト）
			GetBasename( dbcors[res], kMaxPathLen, doc_name );
		}
		else {
			strcpy( doc_name, "reject" );
		}
	} else {
		strcpy( doc_name, "reject" );
	}
	printf( "Result: %s\n\n", doc_name );

	//cvShowImage( "Capture", connect );
	//cvWaitKey(0);
	
	free( ps );
	free( areas );
	free( score );
	ClearNears( num );

	// 検索結果の送信
	SendResult( sock, doc_name );

	return 0;
}

void retrieve_thread( void *arg )
// 受信，検索，送信
{
	IplImage *cap_img;

	// ウィンドウの生成
	cvNamedWindow( "Capture", CV_WINDOW_AUTOSIZE );

	// 撮影画像を受信
	//cap_img = RecvCapImg( ((svTheadArg*)arg)->sock);
	cvShowImage( "Capture", cap_img );
	cvWaitKey( 0 );

	cvReleaseImage(&cap_img);
	cvDestroyWindow( "Capture" );
}

//void *retrieve_thread( void *arg )
//// 受信，検索，送信
//{
//	IplImage *cap_img;
//
//	// ウィンドウの生成
//	cvNamedWindow( "Capture", CV_WINDOW_AUTOSIZE );
//
//	// 撮影画像を受信
//	cap_img = RecvCapImg( ((svTheadArg*)arg)->sock);
//	cvShowImage( "Capture", cap_img );
//	cvWaitKey( 0 );
//
//	cvReleaseImage(&cap_img);
//	cvDestroyWindow( "Capture" );
//
//	return NULL;
//}

void RetrieveSmartphoneServer( void )
{
	SOCKET sock1=0, sock2=0;
	//pthread_t worker ;
	svTheadArg *arg;
	strDisc disc;
	strHash **hash;
	CvPoint **reg_pss=NULL;
	CvSize *reg_sizes=NULL;
	double **reg_areass=NULL;
	int *reg_nums=NULL, ret=0;
	char **dbcors=NULL;

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
	// 組み合わせの作成
	GenerateCombination( eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
	if ( eRmRedund )	LoadRmRedund();	// 特徴選択有

ready:

#ifdef WIN32
	if ( ( sock1 = InitWinSockSvTCP( eTCPPort ) ) < 0 )	exit(1);	// initialization
#else
	if ( ( sock1 = InitSockSvTCP( eTCPPort, eServerName ) ) < 0 )	exit(1);	// initialization
	if ( ( sock2 = AcceptSockSvTCP( sock1 ) ) < 0 )	exit(1);	// accept
#endif
	//// ウィンドウの生成
	//cvNamedWindow( "Capture", CV_WINDOW_AUTOSIZE );
	puts("Ready");
	if ( ( sock2 = AcceptWinSockSvTCP( sock1 ) ) < 0 ) {
		perror( "accept" );
		exit(1);	// accept
	}

	while (1) {

		//tcp_peeraddr_print( sock2 );

		//RecvCapImg( sock2 );
		ret = RetrieveDocImg( sock2, reg_pss, reg_areass, reg_nums, reg_sizes, dbcors, disc, hash );
		if ( ret < 0 )	goto fin;
	}

	//cvDestroyWindow( "Capture" );
fin:
#ifdef	WIN32
	CloseWinSock( sock2 );	// close
#else
	ShutdownSockSvTCP( sock2 );	// shutdown
	CloseSock( sock1 );	// close
#endif
	goto ready;
}

//void RetrieveSmartphoneServer( void )
//{
//	SOCKET sock1=0, sock2=0;
//	pthread_t worker ;
//	svTheadArg *arg;
//
//#ifdef WIN32
//	if ( ( sock1 = InitWinSockSvTCP( eTCPPort ) ) < 0 )	exit(1);	// initialization
//#else
//	if ( ( sock1 = InitSockSvTCP( eTCPPort, eServerName ) ) < 0 )	exit(1);	// initialization
//	if ( ( sock2 = AcceptSockSvTCP( sock1 ) ) < 0 )	exit(1);	// accept
//#endif
//
//	while (1) {
//		if ( ( sock2 = AcceptWinSockSvTCP( sock1 ) ) < 0 ) {
//			perror( "accept" );
//			exit(1);	// accept
//		}
//		tcp_peeraddr_print( sock2 );
//
//		arg = (svTheadArg *)malloc( sizeof(svTheadArg) );
//		if( arg == NULL ) {
//			perror("malloc()");
//			exit( -1 );
//		}
//		arg->sock = sock2;
//
//		if( pthread_create( &worker, NULL, retrieve_thread, (void *)arg) != 0 ) {
//			perror("pthread_create()");
//			exit( 1 );
//		}
//		pthread_detach( worker );
//	}
//
//
//#ifdef	WIN32
//	CloseWinSock( sock2 );	// close
//#else
//	ShutdownSockSvTCP( sock2 );	// shutdown
//	CloseSock( sock1 );	// close
//#endif
//
//}