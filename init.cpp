#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "def_general.h"
#include "extern.h"
#include "fpath.h"

#ifdef WIN32
double log2( double x )
// ���R�ΐ�����2�̑ΐ������߂�
{
	return log( x ) / log( 2.0L );
}
#endif

int CalcnCr( int n, int r )
// nCr���v�Z
{
	int i, num, den;
	for ( i = 0, num = 1, den = 1; i < r; i++ ) {
		num *= n - i;
		den *= i + 1;
	}
	return (int)(num / den);
}

int AnalyzeArgAndSetExtern( int argc, char *argv[] )
// ������͏���
{
	int argi=0;

	// �O���[�o���ϐ��̎��̉�
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
	//eHashSize = (unsigned long long int)1024*1024*1024*4-1;
	eHashStorageNum = 0;
	eThinNum = 0;
	eMultiVecNum = 1;
	eTrueHist = 0;
	eThresh = 0;
	eReHash = 0;
	eBlockSize = 53;
	eC = 5.0;
	eMask = 15;
	eFeatureNum = 400;
	eSampling = 0;

	// �������
	for ( argi = 1; argi < argc && *(argv[argi]) == '-'; argi++ ) {
		switch ( *(argv[argi]+1) ) {	// '-'�̎��̕����ŕ���
			case 'n':	// n
				if ( ++argi < argc )	eGroup1Num = atoi( argv[argi] );
				break;
			case 'm':	// m
				if ( ++argi < argc )	eGroup2Num = atoi( argv[argi] );
				break;
			case 'd':	// ���U�����x��
				if ( ++argi < argc )	eDiscNum = atoi( argv[argi] );
				break;
			case 'c':	// �����_�t�@�C���̍쐬����̃n�b�V���\�z
				eEntireMode = CONST_HASH_MODE;
				if ( ++argi < argc )	strcpy( eSrcDir, argv[argi] );
				if ( ++argi < argc )	strcpy( eSrcSuffix, argv[argi] );
				break;
			case 'x':	// point.dat��p���Ẵn�b�V���\�z
				eEntireMode = CONST_HASH_PF_MODE;
				break;
			case 'h':	// �f�[�^�x�[�X�̃f�B���N�g���w��
				if ( ++argi < argc )	strcpy( eDataDir, argv[argi] );
				break;
			case 'U':	// USB�J�����T�[�o���[�h
				eEntireMode = USBCAM_SERVER_MODE;
				break;
			case 'S':	// �X�}�[�g�t�H���A�g�T�[�o���[�h
				eEntireMode = SMART_SERVER_MODE;
				break;
			case 'p':	// �n�b�V�����̃f�B���N�g�����w��
				if ( ++argi < argc )	strcpy( ePFPrefix, argv[argi] );
				break;
			case 'L':	// �����_�t�@�C���������[�h
				eEntireMode = COMBINE_PF_MODE;
				if ( ++argi < argc )	eCombineStart = atoi( argv[argi] );
				if ( ++argi < argc )	eCombineEnd = atoi( argv[argi] );
				break;
			case 'C':	// ��K�̓n�b�V���������[�h
				eEntireMode = COMBINE_DB_MODE;
				if ( ++argi < argc )	strcpy( eCombineParentDir, argv[argi] );
				if ( ++argi < argc )	eCombineStart = atoi( argv[argi] );
				if ( ++argi < argc )	eCombineEnd = atoi( argv[argi] );
				break;
			case 'P':	// ��K�̓n�b�V���������[�h
				eEntireMode = CONST_POINT_MODE;
				if ( ++argi < argc )	strcpy( eSrcDir, argv[argi] );
				if ( ++argi < argc )	strcpy( eSrcSuffix, argv[argi] );
				break;
			case 'H':	// ��K�̓n�b�V���������[�h
				eEntireMode = COMBINE_HASH_MODE;
				break;
			case 'q':	// �N�G�������_�t�@�C���쐬���[�h
				eEntireMode = MAKE_QUERY_DAT_MODE;
				break;
			case 'f':	// �����I��
				eEntireMode = COMPRESS_PF_MODE;
				break;
			case 'e':	// �������[�h
				eExperimentMode = 1;
				break;
			case 'j':	// ���ڃn�b�V���������[�h
				eDirectHashCombine = 1;
				break;
			case 'a':	// �����ʌv�Z�ɖʐς𗘗p
				eUseArea = 1;
				break;
			case 'z':	// ���X�g���k���[�h
				eCompressHash = 1;
				break;
			case 'b':	// ���U���t�@�C���ǂݍ��݃n�b�V���\�z
				eLoadDiscHash = 1;
				break;
			case 'M':	// �}���`�v���[�u����
				eRetMultiVec = 1;
				if ( ++argi < argc )	eBounds = atoi( argv[argi] );
				if ( ++argi < argc )	eMultiNum = atoi( argv[argi] );
				break;
			case 'r':	// �����I��
				eRmRedund = 1;
				eGroup1Num = 8;
				eGroup2Num = 7;
				break;
			case 'o':	// �����I��
				eTrueHist = 1;
				break;
			case 'i':	// �����I��
				eThresh = 1;
				if ( ++argi < argc )	eFeatureNum = atoi( argv[argi] );
				break;
			case 'k':	// �����I��
				eReHash = 1;
				break;
			case 's':
				eSampling = 1;
				break;
			case 't':	// �����_�T���v�����O
				eThinPs = 1;
				if ( ++argi < argc )	eThinNum = atoi( argv[argi] );
				break;
			case 'g':	// �����_�T���v�����O
				if ( ++argi < argc )	eBlockSize = atoi( argv[argi] );
				if ( ++argi < argc )	eC = atof( argv[argi] );
				if ( ++argi < argc )	eMask = atoi( argv[argi] );
				break;
			case 'l':	// �ő�Փˉ�
				if ( ++argi < argc )	eMaxHashCollision = atoi( argv[argi] );
				break;
			default:	// ���m�̈���
				fprintf( stderr, "warning: %c is an unknown argument\n",  *(argv[argi]+1) );
				break;
		}
	}

	// �f�B���N�g���̖����Ɂi�Ȃ���΁j�X���b�V����ǉ�
	AddSlash( eSrcDir, kMaxPathLen );
	AddSlash( eDataDir, kMaxPathLen );
	AddSlash( ePFPrefix, kMaxPathLen );
	AddSlash( eCombineParentDir, kMaxPathLen );

	// �g�ݍ��킹�����v�Z
	if ( eGroup1Num < eGroup2Num || eGroup2Num < eGroup3Num ) {
		fprintf( stderr, "error: illegal n or m\n" );
		exit(1);
	}
	eNumCom1 = CalcnCr( eGroup1Num, eGroup2Num );
	eNumCom2 = CalcnCr( eGroup2Num, eGroup3Num );
	eFByte = sizeof(int) + sizeof(short) + sizeof(unsigned char);	// ����ID�C�����_ID�C�������킹������

	eDocBit = (int)ceil( log2(kMaxDocNum) );	// doc�̃r�b�g��
	ePointBit = (int)ceil( log2(kMaxPointNum) );
	eQuotBit = 8;
	eHComp2DatByte = (int)ceil( ((double)( eDocBit + ePointBit + eQuotBit )) / 8.0L );	// ���X�g�̃f�[�^���̃o�C�g��

	//printf("eHComp2DatByte:[%d]\n", eHComp2DatByte);

	if ( eRetMultiVec ) {
		eMultiVecNum = eMultiVecNum << 17;
	}

	return argi;
}

int ReadIniFile( void )
// ini�t�@�C����ǂ�
{
	char line[kMaxLineLen], *tok;
	FILE *fp;

	// �f�t�H���g�l�̐ݒ�
	eTCPPort = kDefaultTCPPort;
	eProtocol = kDefaultProtocol;
	ePointPort = kDefaultPointPort;
	eResultPort = kDefaultResultPort;
	strcpy( eClientName, kDefaultClientName );
	strcpy( eServerName, kDefaultServerName );

	if ( ( fp = fopen( kIniFileName, "r" ) ) == NULL ) {	// ini�t�@�C�����Ȃ�
		fprintf( stderr, "warning: %s cannot be opened\n", kIniFileName );
		return 0;
	}
	for ( ; fgets( line, kMaxLineLen, fp ) != NULL;  ) {	// ini�t�@�C�����s���Ƃɏ���
//		puts( line );
		tok = strtok( line, " =\t\n" );	// strtok�ōs�𕪉�
		if ( tok == NULL )	continue;	// �g�[�N���Ȃ�
		if ( *tok == '#' )	continue;	// �s����#�Ȃ�R�����g�Ƃ݂Ȃ�
		if ( strcmp( tok, "TCPPort" ) == 0 ) {
			tok = strtok( NULL, " =\t\n" );
			if ( tok != NULL )	eTCPPort = atoi(tok);
		}
		else if ( strcmp( tok, "Protocol" ) == 0 ) {	// Protocol
			tok = strtok( NULL, " =\t\n" );
			if ( tok == NULL )	continue;	// �g�[�N���Ȃ�
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
