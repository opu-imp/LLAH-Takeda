#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef	WIN32
	#include <windows.h>
	#include <mmsystem.h>
	#include <sys/timeb.h>
#else
	#include <glob.h>
#endif

#include "def_general.h"

int AddSlash( char *path, int len )
// ������path�̖�����/���Ȃ���Α}������
{
	int i;

	// ������̒����𒲂ׂ�
	for ( i = 0; i < len && path[i] != '\0'; i++ );
	// �I�[������������Ȃ������ꍇ
	if ( i == len )	return 0;
	// �I�[������len-1�ɂ���C�ǉ��ł��Ȃ��ꍇ
	if ( i == len - 1 && path[i-1] != kDelimiterChar )	return 0;
	// ���ł�'/'������ꍇ
	if ( path[i-1] == kDelimiterChar )	return 1;

	// '/'��ǉ�
	path[i] = kDelimiterChar;
	path[i+1] = '\0';

	return 1;
}

int errfunc_fpath( const char *epath, int eerrno )
{
	fprintf(stderr, "%d : %s\n", eerrno, epath );
	return 1;
}

int GetDir( char *path, int len, char *dir )
// �p�X������path����f�B���N�g���̕�������擾���Cdir�Ɋi�[����
{
	int i, last_slash;
	
	for ( i = 0, last_slash = -1; i < len && path[i] != '\0'; i++ ) {
		dir[i] = path[i];
		if ( path[i] == kDelimiterChar )	last_slash = i;
	}
	// len�܂ŏI�[������������Ȃ������ꍇ
	if ( i == len )	return 0;
	// �X���b�V����������Ȃ������ꍇ�A�󕶎����Ԃ�
	if ( last_slash < 0 ) {
		dir[0] = '\0';
		return 1;
	}
	dir[last_slash + 1] = '\0';
	return 1;
}


int FindPath( char search_path[kMaxPathLen], char ***files0 )
// �p�X��W�J����
{
	int i, num;
	char **files;
#ifdef	WIN32
	int count, wlen;
	char dir[kMaxPathLen], fname[kMaxPathLen];
	WIN32_FIND_DATA fFind;
	HANDLE hSearch;
	BOOL ret = TRUE;
	WCHAR wpath[kMaxPathLen];
	size_t convertedChars = 0;
#else
	glob_t gt;
	int ret;
#endif

#ifdef	WIN32
	GetDir( search_path, kMaxPathLen, dir );	// �f�B���N�g�����擾
	mbstowcs_s( &convertedChars, wpath, strlen(search_path) + 1, search_path, _TRUNCATE );	// WCHAR�ɕϊ�
	hSearch = FindFirstFile( wpath, &fFind); /* ���摜�f�B���N�g���̒T���J�n */
	if ( hSearch == INVALID_HANDLE_VALUE ) {	// �����Ɏ��s
		fprintf( stderr, "error: %s matches no files\n", search_path );
		return 0;
	}
	// ���𐔂���
	num = 0;
	do {
		num++;
		ret = FindNextFile( hSearch, &fFind );
	} while ( ret == TRUE );
	FindClose( hSearch );
	// ������x�������ăt�@�C�������i�[����
	hSearch = FindFirstFile( wpath, &fFind); /* ���摜�f�B���N�g���̒T���J�n */
	files = (char **)calloc( num, sizeof(char *) );
	for ( i = 0, count = 0; i < num; i++ ) {
		wcstombs_s( &convertedChars, fname, kMaxPathLen, fFind.cFileName, _TRUNCATE );	// �Ƃ肠�����ϊ�
		if ( convertedChars > 0 ) {	// �ϊ������Ȃ�
			wlen = wcslen(fFind.cFileName);
			files[count] = (char *)calloc( strlen(dir) + wlen + 1, sizeof(char) );
			strcpy( files[count], dir );	// �Ƃ肠�����f�B���N�g�����R�s�[
			strcat( files[count], fname );	// �t�@�C����������
			count++;
		}
		else {	// �ϊ����s
			fprintf( stderr, "warning: file name conversion failed\n" );
		}

		FindNextFile( hSearch, &fFind );
	}
	FindClose( hSearch );
	*files0 = files;
	return count;
#else
	ret = glob( search_path, 0, errfunc_fpath, &gt );	// glob�̊J�n
	if ( ret != 0 ) {
		fprintf( stderr, "error: glob() returned %d\n", ret );
		return 0;
	}
	num = (int)gt.gl_pathc;
	files = (char **)calloc( num, sizeof(char *) );
	for ( i = 0; i < num; i++ ) {
		files[i] = (char *)calloc( strlen(gt.gl_pathv[i]) + 1, sizeof(char) );
		strcpy( files[i], gt.gl_pathv[i] );
	}
	globfree( &gt );
#endif
	*files0 = files;
	return num;
}

int GetBasename( char *path, int len, char *basename )
// �p�X������path����f�B���N�g�������Ɗg���q����������������擾���Cbasename�Ɋi�[����
{
	int i, last_slash, first_dot;
	
	basename[0] = '\0';
	for ( i = 0, last_slash = -1, first_dot = -1; i < len && path[i] != '\0'; i++ ) {
		if ( path[i] == kDelimiterChar )	last_slash = i;
		if ( path[i] == '.'/*&& first_dot < 0*/ )	first_dot = i;
	}
	// len�܂ŏI�[������������Ȃ������ꍇ
	if ( i == len )	return 0;
	// �h�b�g���Ȃ������ꍇ
	last_slash++;
	if ( first_dot < 0 )	first_dot = i - 1;
	strcpy( basename, path + last_slash );
	basename[first_dot - last_slash] = '\0';
	
	return 1;
}

int IsDat( char *str )
// str�̖�����.dat�����ׂ�
{
	int i, last_dot = -1;
	
	for ( i = 0; str[i] != '\0'; i++ ) {
		if ( str[i] == '.' )	last_dot = i;
	}
	if ( last_dot < 0 )	return 0;
	if ( !strcmp( str+last_dot+1, "dat" ) || !strcmp( str+last_dot+1, "DAT" ) )	return 1;
	else	return 0;
}
