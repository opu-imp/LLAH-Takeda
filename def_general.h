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

#define	kFileNameLen	(64)	/* ファイル名の最大長 */
#define	kMaxPathLen		(128)	/* パスの文字列の最大長 */
#define	kMaxLineLen		(1024)	/* fgetsで読み込む場合のバッファサイズ */
#define	kDiscLineLen	(1024)	/* fgetsで読み込む場合のバッファサイズ */
#define kBuffSize		(1024)	/* TCP/IP通信でのバッファサイズ */

#define PI (3.14159265358979323846L)

#ifdef WIN32
#ifndef min
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#endif
#ifndef	max
#define	max(a, b)	((a > b) ? a : b);
#endif
#endif

// ネットワーク関連
#define	kIniFileName	"server.ini"
#define	kDefaultTCPPort		(12345)
#define	kDefaultProtocol	(2)	/* 1はTCP，2はUDP */
#define	kDefaultPointPort	(65431)
#define	kDefaultResultPort	(65432)
#define	kDefaultClientName	"localhost"
#define	kDefaultServerName	"localhost"

// ファイルオープン関連
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

// 離散化
#define	kDfltDocNumForMakeDisc	(10)

// 比例定数
#define	kDfltPropMakeNum	(10)

// 一般
#define	kConfigVerStr	"<config.dat 2.0>"

// ハッシュの構築
#define	kMaxHashCollision	(10)	// 最大衝突数
#define	kMaxDocNum	(1024*1024*128)	/* 登録できる最大ドキュメント数 */
#define	kMaxPointNum	(2500)	/* 点の最大数 */
//#define	kHashSize	(1024*1024*1024*2-1)	/* ハッシュテーブルの大きさ */
//#define	kHashSize	(1024*1024*1024-1)	/* ハッシュテーブルの大きさ */
//#define	kHashSize	(1024*1024*8-1)	/* ハッシュテーブルの大きさ */
#define	kHashSize	(1024*1024*128-1)	/* ハッシュテーブルの大きさ */
#define	kBlockSize	(1000000)
//#define	kBlockSize	(1024)

// 全体のモード
#define	RETRIEVE_MODE		(0)		// 通常検索モード
#define	CONST_HASH_MODE		(1)		// ハッシュ構築モード
#define CONST_HASH_PF_MODE	(2)		// 点ファイルからのハッシュ構築モード
#define	USBCAM_SERVER_MODE	(3)		// リアルタイム検索モード
#define COMBINE_PF_MODE		(4)		// 
#define COMBINE_DB_MODE		(5)
#define COMBINE_HASH_MODE	(6)
#define MAKE_QUERY_DAT_MODE (7)
#define SMART_SERVER_MODE	(8)
#define CONST_POINT_MODE	(9)
#define COMPRESS_PF_MODE	(10)

// パス操作
#define kDelimiterChar '/'

// カラー
#define	cWhite	CV_RGB( 255,255,255 )
#define	cBlack	CV_RGB( 0,0,0 )
#define	cRed	CV_RGB( 255,0,0 )
#define	cGreen	CV_RGB( 0,255,0 )
#define	cBlue	CV_RGB( 0,0,255 )
#define	cRandom	CV_RGB( rand()%256, rand()%256, rand()%256 )

// 連続値のアフィン不変量を離散値に変換するための構造体
typedef struct _strDisc {
	double min;	// 最小値
	double max;	// 最大値
	int num;	// 離散化レベル
	int res;	// datの個数
	int *dat;	// 離散値を入れる配列
} strDisc;

//アフィン不変量のヒストグラム構築用の構造体
typedef struct _strHist {
	int size;	// binの数
	int *bin;	// データ
	double min;	// データの最小値
	double max;	// データの最大値．minとmaxで入るbinを決める
} strHist;

typedef unsigned char strHash; // 配列でハッシュ構成

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
