#ifdef	WIN32
#pragma	warning(disable:4996)
#pragma	warning(disable:4819)
#endif

#ifdef	WIN32
#define	WIN_TIME
#else	
#define LINUX_TIME
#endif

#ifndef	WIN32
#define	SOCKET	int
#endif

#define	kFileNameLen	(64)	/* �t�@�C�����̍ő咷 */
#define	kMaxPathLen		(128)	/* �p�X�̕�����̍ő咷 */
#define	kMaxLineLen		(1024)	/* fgets�œǂݍ��ޏꍇ�̃o�b�t�@�T�C�Y */
#define	kDiscLineLen	(1024)	/* fgets�œǂݍ��ޏꍇ�̃o�b�t�@�T�C�Y */
#define kBuffSize		(1024)	/* TCP/IP�ʐM�ł̃o�b�t�@�T�C�Y */

#define PI (3.14159265358979323846L)

#ifdef WIN32
#ifndef min
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#endif
#ifndef	max
#define	max(a, b)	((a > b) ? a : b);
#endif
#endif

// �l�b�g���[�N�֘A
#define	kIniFileName	"server.ini"
#define	kDefaultTCPPort		(12345)
#define	kDefaultProtocol	(2)	/* 1��TCP�C2��UDP */
#define	kDefaultPointPort	(65431)
#define	kDefaultResultPort	(65432)
#define	kDefaultClientName	"localhost"
#define	kDefaultServerName	"localhost"

// �t�@�C���I�[�v���֘A
#define	kDfltDataDir	"./"
#define	kDfltSrcDir	"./"
#define	kDfltSrcSuffix	"jpg"
#define	kDfltPointFileName	"point.dat"
#define	kDfltDiscFileName	"disc.dat"
#define	kDfltHashFileName	"hash.dat"
#define	kDfltConfigFileName	"config.dat"
#define	kDfltThinFileName	"thin.dat"
#define	kCopyright	"Copyright (C) 2006 Tomohiro Nakai, Intelligent Media Processing Laboratory, Osaka Prefecture University"

#define	kDfltGroup1Num	(7)
#define	kDfltGroup2Num	(6)
#define	kDfltGroup3Num	(4)
#define	kDfltDiscNum	(15)

// ���U��
#define	kDfltDocNumForMakeDisc	(10)

// ���萔
#define	kDfltPropMakeNum	(10)

// ���
#define	kConfigVerStr	"<config.dat 2.0>"

// �n�b�V���̍\�z
#define	kMaxHashCollision	(10)	// �ő�Փː�
#define	kMaxDocNum	(1024*1024*128)	/* �o�^�ł���ő�h�L�������g�� */
#define	kMaxPointNum	(2500)	/* �_�̍ő吔 */
//#define	kHashSize	(1024*1024*1024*2-1)	/* �n�b�V���e�[�u���̑傫�� */
//#define	kHashSize	(1024*1024*1024-1)	/* �n�b�V���e�[�u���̑傫�� */
//#define	kHashSize	(1024*1024*8-1)	/* �n�b�V���e�[�u���̑傫�� */
#define	kHashSize	(1024*1024*128-1)	/* �n�b�V���e�[�u���̑傫�� */
#define	kBlockSize	(1000000)
//#define	kBlockSize	(1024)

// �S�̂̃��[�h
#define	RETRIEVE_MODE		(0)		// �ʏ팟�����[�h
#define	CONST_HASH_MODE		(1)		// �n�b�V���\�z���[�h
#define CONST_HASH_PF_MODE	(2)		// �_�t�@�C������̃n�b�V���\�z���[�h
#define	USBCAM_SERVER_MODE	(3)		// ���A���^�C���������[�h
#define COMBINE_PF_MODE		(4)		// 
#define COMBINE_DB_MODE		(5)
#define COMBINE_HASH_MODE	(6)
#define MAKE_QUERY_DAT_MODE (7)
#define SMART_SERVER_MODE	(8)
#define CONST_POINT_MODE	(9)
#define COMPRESS_PF_MODE	(10)

// パス区切り
#define kDelimiterChar '/'

// OpenCV (for types used in headers)
#include <opencv2/core.hpp>

// カラー
#define	cWhite	cv::Scalar( 255,255,255 )
#define	cBlack	cv::Scalar( 0,0,0 )
#define	cRed	cv::Scalar( 0,0,255 )
#define	cGreen	cv::Scalar( 0,255,0 )
#define	cBlue	cv::Scalar( 255,0,0 )
#define	cRandom	cv::Scalar( rand()%256, rand()%256, rand()%256 )

// �A���l�̃A�t�B���s�ϗʂ𗣎U�l�ɕϊ����邽�߂̍\����
typedef struct _strDisc {
	double min;	// �ŏ��l
	double max;	// �ő�l
	int num;	// ���U�����x��
	int res;	// dat�̌�
	int *dat;	// ���U�l������z��
} strDisc;

//�A�t�B���s�ϗʂ̃q�X�g�O�����\�z�p�̍\����
typedef struct _strHist {
	int size;	// bin�̐�
	int *bin;	// �f�[�^
	double min;	// �f�[�^�̍ŏ��l
	double max;	// �f�[�^�̍ő�l�Dmin��max�œ���bin�����߂�
} strHist;

typedef unsigned char strHash; // �z��Ńn�b�V���\��

typedef struct _strProjParam {
	double a1;
	double a2;
	double a3;
	double b1;
	double b2;
	double b3;
	double c1;
	double c2;
} strProjParam;


typedef struct _svThreadArg {
	int sock;
} svTheadArg;
