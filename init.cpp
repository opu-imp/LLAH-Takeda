#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "def_general.h"
#include "extern.h"
#include "fpath.h"

#ifdef WIN32
double log2( double x )
// 自然対数から2の対数を求める
{
	return log( x ) / log( 2.0L );
}
#endif

int CalcnCr( int n, int r )
// nCrを計算
{
	int i, num, den;
	for ( i = 0, num = 1, den = 1; i < r; i++ ) {
		num *= n - i;
		den *= i + 1;
	}
	return (int)(num / den);
}

int AnalyzeArgAndSetExtern( int argc, char *argv[] )
// 引数解析処理
{
	int argi=0;

	// グローバル変数の実体化
	strcpy( eSrcDir, kDfltSrcDir );
	strcpy( eSrcSuffix, kDfltSrcSuffix );
	strcpy( ePointFileName, kDfltPointFileName );
	strcpy( eDiscFileName, kDfltDiscFileName );
	strcpy( eHashFileName, kDfltHashFileName );
	strcpy( eConfigFileName, kDfltConfigFileName );
	strcpy( eThinFileName, kDfltThinFileName );
	eGroup1Num = kDfltGroup1Num;
	eGroup2Num = kDfltGroup2Num;
	eGroup3Num = kDfltGroup3Num;
	eDiscNum = kDfltDiscNum;
	eDiscRes = 1000;
	//eDiscMin = 0.0;
	//eDiscMax = 10.0;
	eDiscMin = 0.0;
	eDiscMax = 10.0;
	eMaxHashCollision = kMaxHashCollision;
	eDocNumForMakeDisc = kDfltDocNumForMakeDisc;
	ePropMakeNum = kDfltPropMakeNum;
	eEntireMode = RETRIEVE_MODE;
	eUseArea = 0;
	eHashSize = (unsigned long long int)1024*1024*512-1;
	//eHashSize = 17179869184-1;
	//eHashSize = (unsigned long long int)1024*1024*1024*4-1;
	eHashStorageNum = 0;
	eThinNum = 0;
	eMultiVecNum = 1;
	eTrueHist = 0;
	eThresh = 0;
	eReHash = 0;
	eBlockSize = 53;
	eC = 5.0;
	eFeatureNum = 400;
	eSampling = 0;

	// 引数解析
	for ( argi = 1; argi < argc && *(argv[argi]) == '-'; argi++ ) {
		switch ( *(argv[argi]+1) ) {	// '-'の次の文字で分岐
			case 'n':	// n
				if ( ++argi < argc )	eGroup1Num = atoi( argv[argi] );
				break;
			case 'm':	// m
				if ( ++argi < argc )	eGroup2Num = atoi( argv[argi] );
				break;
			case 'd':	// 離散化レベル
				if ( ++argi < argc )	eDiscNum = atoi( argv[argi] );
				break;
			case 'c':	// 特徴点ファイルの作成からのハッシュ構築
				eEntireMode = CONST_HASH_MODE;
				if ( ++argi < argc )	strcpy( eSrcDir, argv[argi] );
				if ( ++argi < argc )	strcpy( eSrcSuffix, argv[argi] );
				break;
			case 'x':	// point.datを用いてのハッシュ構築
				eEntireMode = CONST_HASH_PF_MODE;
				break;
			case 'h':	// データベースのディレクトリ指定
				if ( ++argi < argc )	strcpy( eDataDir, argv[argi] );
				break;
			case 'U':	// USBカメラサーバモード
				eEntireMode = USBCAM_SERVER_MODE;
				break;
			case 'S':	// スマートフォン連携サーバモード
				eEntireMode = SMART_SERVER_MODE;
				break;
			case 'p':	// ハッシュ等のディレクトリを指定
				if ( ++argi < argc )	strcpy( ePFPrefix, argv[argi] );
				break;
			case 'L':	// 特徴点ファイル結合モード
				eEntireMode = COMBINE_PF_MODE;
				if ( ++argi < argc )	eCombineStart = atoi( argv[argi] );
				if ( ++argi < argc )	eCombineEnd = atoi( argv[argi] );
				break;
			case 'C':	// 大規模ハッシュ結合モード
				eEntireMode = COMBINE_DB_MODE;
				if ( ++argi < argc )	strcpy( eCombineParentDir, argv[argi] );
				if ( ++argi < argc )	eCombineStart = atoi( argv[argi] );
				if ( ++argi < argc )	eCombineEnd = atoi( argv[argi] );
				break;
			case 'P':	// 大規模ハッシュ結合モード
				eEntireMode = CONST_POINT_MODE;
				if ( ++argi < argc )	strcpy( eSrcDir, argv[argi] );
				if ( ++argi < argc )	strcpy( eSrcSuffix, argv[argi] );
				break;
			case 'H':	// 大規模ハッシュ結合モード
				eEntireMode = COMBINE_HASH_MODE;
				break;
			case 'q':	// クエリ特徴点ファイル作成モード
				eEntireMode = MAKE_QUERY_DAT_MODE;
				break;
			case 'f':	// 得著選択
				eEntireMode = COMPRESS_PF_MODE;
				break;
			case 'e':	// 実験モード
				eExperimentMode = 1;
				break;
			case 'j':	// 直接ハッシュ結合モード
				eDirectHashCombine = 1;
				break;
			case 'a':	// 特徴量計算に面積を利用
				eUseArea = 1;
				break;
			case 'z':	// リスト圧縮モード
				eCompressHash = 1;
				break;
			case 'b':	// 離散化ファイル読み込みハッシュ構築
				eLoadDiscHash = 1;
				break;
			case 'M':	// マルチプルーブ検索
				eRetMultiVec = 1;
				if ( ++argi < argc )	eBounds = atoi( argv[argi] );
				if ( ++argi < argc )	eMultiNum = atoi( argv[argi] );
				break;
			case 'r':	// 得著選択
				eRmRedund = 1;
				eGroup1Num = 8;
				eGroup2Num = 7;
				break;
			case 'o':	// 得著選択
				eTrueHist = 1;
				break;
			case 'i':	// 得著選択
				eThresh = 1;
				if ( ++argi < argc )	eFeatureNum = atoi( argv[argi] );
				break;
			case 'k':	// 得著選択
				eReHash = 1;
				break;
			case 's':
				eSampling = 1;
				break;
			case 't':	// 特徴点サンプリング
				eThinPs = 1;
				if ( ++argi < argc )	eThinNum = atoi( argv[argi] );
				break;
			case 'g':	// 特徴点サンプリング
				if ( ++argi < argc )	eBlockSize = atoi( argv[argi] );
				if ( ++argi < argc )	eC = atof( argv[argi] );
				if ( ++argi < argc )	eMask = atoi( argv[argi] );
				break;
			case 'l':	// 最大衝突回数
				if ( ++argi < argc )	eMaxHashCollision = atoi( argv[argi] );
				break;
			default:	// 未知の引数
				fprintf( stderr, "warning: %c is an unknown argument\n",  *(argv[argi]+1) );
				break;
		}
	}

	// ディレクトリの末尾に（なければ）スラッシュを追加
	AddSlash( eSrcDir, kMaxPathLen );
	AddSlash( eDataDir, kMaxPathLen );
	AddSlash( ePFPrefix, kMaxPathLen );
	AddSlash( eCombineParentDir, kMaxPathLen );

	// 組み合わせ数を計算
	if ( eGroup1Num < eGroup2Num || eGroup2Num < eGroup3Num ) {
		fprintf( stderr, "error: illegal n or m\n" );
		exit(1);
	}
	eNumCom1 = CalcnCr( eGroup1Num, eGroup2Num );
	eNumCom2 = CalcnCr( eGroup2Num, eGroup3Num );
	eFByte = sizeof(int) + sizeof(short) + sizeof(unsigned char);	// 文書ID，特徴点ID，商を合わせた長さ

	eDocBit = (int)ceil( log2(kMaxDocNum) );	// docのビット数
	ePointBit = (int)ceil( log2(kMaxPointNum) );
	eQuotBit = 8;
	eHComp2DatByte = (int)ceil( ((double)( eDocBit + ePointBit + eQuotBit )) / 8.0L );	// リストのデータ部のバイト数

	//printf("eHComp2DatByte:[%d]\n", eHComp2DatByte);

	if ( eRetMultiVec ) {
		eMultiVecNum = eMultiVecNum << 17;
	}

	return argi;
}

int ReadIniFile( void )
// iniファイルを読む
{
	char line[kMaxLineLen], *tok;
	FILE *fp;

	// デフォルト値の設定
	eTCPPort = kDefaultTCPPort;
	eProtocol = kDefaultProtocol;
	ePointPort = kDefaultPointPort;
	eResultPort = kDefaultResultPort;
	strcpy( eClientName, kDefaultClientName );
	strcpy( eServerName, kDefaultServerName );

	if ( ( fp = fopen( kIniFileName, "r" ) ) == NULL ) {	// iniファイルがない
		fprintf( stderr, "warning: %s cannot be opened\n", kIniFileName );
		return 0;
	}
	for ( ; fgets( line, kMaxLineLen, fp ) != NULL;  ) {	// iniファイルを行ごとに処理
//		puts( line );
		tok = strtok( line, " =\t\n" );	// strtokで行を分解
		if ( tok == NULL )	continue;	// トークンなし
		if ( *tok == '#' )	continue;	// 行頭が#ならコメントとみなす
		if ( strcmp( tok, "TCPPort" ) == 0 ) {
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL )	eTCPPort = atoi(tok);
		}
		else if ( strcmp( tok, "Protocol" ) == 0 ) {	// Protocol
			tok = strtok( NULL, " =\t\n" );
			if ( tok == NULL )	continue;	// トークンなし
			if ( strcmp( tok, "TCP" ) == 0 )	eProtocol = 1;
			else if ( strcmp( tok, "UDP" ) == 0 )	eProtocol = 2;
		}
		else if ( strcmp( tok, "PointPort" ) == 0 ) {
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL )	ePointPort = atoi( tok );
		}
		else if ( strcmp( tok, "ResultPort" ) == 0 ) {
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL )	eResultPort = atoi( tok );
		}
		else if ( strcmp( tok, "ClientName" ) == 0 ) {
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL )	strcpy( eClientName, tok );
		}
		else if ( strcmp( tok, "ServerName" ) == 0 ) {
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL )	strcpy( eServerName, tok );
		}
	}
	fclose( fp );

	return 1;
}
