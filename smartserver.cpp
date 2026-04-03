#include "def_general.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// �l�b�g���[�N�֘A
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

//// �}���`�X���b�h
//#include <pthread.h>    /* pthread_create */

// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#ifdef _DEBUG
#pragma comment( lib, "opencv_imgproc4d.lib" )
#pragma comment( lib, "opencv_core4d.lib" )
#pragma comment( lib, "opencv_highgui4d.lib" )
#else
#pragma comment( lib, "opencv_imgproc4.lib" )
#pragma comment( lib, "opencv_core4.lib" )
#pragma comment( lib, "opencv_highgui4.lib" )
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

cv::Mat RecvCapImg( SOCKET sock )
	// �_�f�[�^����M
{
	char size[32];
	char *buff;
	unsigned char **data;
	int buff_size=0, total=0, ret=0, width=0, height=0, buff_cur=0;
	FILE *fp;

	// �B�e�摜�̃T�C�Y����M
	while ( total < 12 ) {
		ret = recv ( sock, &size[total], 12-total, RECV_FLAG );
		total += ret;
	}
	memcpy( &width, &size[0], sizeof(int) );
	memcpy( &height, &size[4], sizeof(int) );
	memcpy( &buff_size, &size[8], sizeof(int) );

	if ( width < 0 )	return cv::Mat();

	//printf("Width: %d, Height: %d\n", width, height);
	//printf("Data Size: %d\n", buff_size);

	buff = (char *)malloc(buff_size);

	// �B�e�摜����M
	total=0;
	while ( total < buff_size ) {
		//printf("%d\n", total);
		ret = recv ( sock, &buff[total], buff_size-total, RECV_FLAG );
		total += ret;
	}
	puts("Recieved");

	// Mat�ւ̕ϊ�
	Mat mat( Size(width, height), CV_8UC1);
	for ( int i=0; i<height; i++ ) {
		for ( int j=0; j<width; j++ ) {
			mat.at<unsigned char>(i,j) = (unsigned char)buff[i*width+j];
		}
	}

	free(buff);

	return mat;
}

void SendResult( SOCKET sock, char *result_name )
{
	send ( sock, result_name, kFileNameLen, SEND_FLAG );
}

int RetrieveDocImg( SOCKET sock, cv::Point **reg_pss, double **reg_areass, int *reg_nums, cv::Size *reg_sizes, char **dbcors, strDisc disc, strHash **hash )
// �X�}�t�H����摜����M���C�����C���ʂ𑗐M
{
	cv::Mat img, connect;
	cv::Point *ps=NULL;
	cv::Size size;
	int num;
	double *areas=NULL;

	int res=0, ret_time=0;
	char doc_name[kFileNameLen]={0};

	// �B�e�摜�̎�M
	img = RecvCapImg( sock );
	if ( img.empty() )	return -1;

	// ���[�e�[�u���m��
	int *score = (int *)calloc(eDbDocs, sizeof(int));
	// �����摜����
	connect = GetConnectedImageForSmartphone( img );
	if ( connect.empty() )	exit(-1);
	// �����_���o�E�A�������̖ʐόv�Z
	num = MakeFeaturePoint( connect, &ps, &areas, &size );
	if ( num >= 20 ) {	// �����_����臒l
		// �ߖT�\���̉��
		MakeNearsFromCentres( ps, num );
		// ����
		res = Retrieve( ps, areas, num, score, &disc, reg_nums, &ret_time, hash );
		if ( score[res] > 10 ) {	// �g�b�v�̓��[����臒l�ȉ��i���W�F�N�g�j
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

	// �������ʂ̑��M
	SendResult( sock, doc_name );

	return 0;
}

void retrieve_thread( void *arg )
// ��M�C�����C���M
{
	cv::Mat cap_img;

	cv::namedWindow( "Capture", cv::WINDOW_AUTOSIZE );

	//cap_img = RecvCapImg( ((svTheadArg*)arg)->sock);
	cv::imshow( "Capture", cap_img );
	cv::waitKey( 0 );

	cv::destroyWindow( "Capture" );
}

//void *retrieve_thread( void *arg )
//// ��M�C�����C���M
//{
//	IplImage *cap_img;
//
//	// �E�B���h�E�̐���
//	cvNamedWindow( "Capture", CV_WINDOW_AUTOSIZE );
//
//	// �B�e�摜����M
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
	cv::Point **reg_pss=NULL;
	cv::Size *reg_sizes=NULL;
	double **reg_areass=NULL;
	int *reg_nums=NULL, ret=0;
	char **dbcors=NULL;

	// �ݒ�t�@�C���̓ǂݍ���
	LoadConfig();
	// ���U���t�@�C���̓ǂݍ���
	LoadDisc( &disc );
	// �n�b�V���̓ǂݍ���
	LoadHashCompressToStorage( &hash );
	// �����_�t�@�C���̓ǂݍ���
	if ( eThinPs )	eDbDocs = LoadThinFile( NULL, &reg_pss, &reg_sizes, &reg_nums, &dbcors );								// �T���v�����O�L��
	else			eDbDocs = LoadPointFile( NULL, &reg_pss, &reg_areass, &reg_sizes, &reg_nums, &dbcors );	// �T���v�����O����
	// �ߖT�\���ۑ��p�������m��
	SecureNears();
	// �g�ݍ��킹�̍쐬
	GenerateCombination( eGroup1Num, eGroup2Num, eGroup3Num, SetCom1, SetCom2 );
	if ( eRmRedund )	LoadRmRedund();	// �����I��L

ready:

#ifdef WIN32
	if ( ( sock1 = InitWinSockSvTCP( eTCPPort ) ) < 0 )	exit(1);	// initialization
#else
	if ( ( sock1 = InitSockSvTCP( eTCPPort, eServerName ) ) < 0 )	exit(1);	// initialization
	if ( ( sock2 = AcceptSockSvTCP( sock1 ) ) < 0 )	exit(1);	// accept
#endif
	//// �E�B���h�E�̐���
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