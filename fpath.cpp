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
// 文字列pathの末尾に/がなければ挿入する
{
	int i;

	// 文字列の長さを調べる
	for ( i = 0; i < len && path[i] != '\0'; i++ );
	// 終端文字が見つからなかった場合
	if ( i == len )	return 0;
	// 終端文字がlen-1にあり，追加できない場合
	if ( i == len - 1 && path[i-1] != kDelimiterChar )	return 0;
	// すでに'/'がある場合
	if ( path[i-1] == kDelimiterChar )	return 1;

	// '/'を追加
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
// パス文字列pathからディレクトリの文字列を取得し，dirに格納する
{
	int i, last_slash;
	
	for ( i = 0, last_slash = -1; i < len && path[i] != '\0'; i++ ) {
		dir[i] = path[i];
		if ( path[i] == kDelimiterChar )	last_slash = i;
	}
	// lenまで終端文字が見つからなかった場合
	if ( i == len )	return 0;
	// スラッシュが見つからなかった場合、空文字列を返す
	if ( last_slash < 0 ) {
		dir[0] = '\0';
		return 1;
	}
	dir[last_slash + 1] = '\0';
	return 1;
}


int FindPath( char search_path[kMaxPathLen], char ***files0 )
// パスを展開する
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
	GetDir( search_path, kMaxPathLen, dir );	// ディレクトリを取得
	mbstowcs_s( &convertedChars, wpath, strlen(search_path) + 1, search_path, _TRUNCATE );	// WCHARに変換
	hSearch = FindFirstFile( wpath, &fFind); /* 元画像ディレクトリの探索開始 */
	if ( hSearch == INVALID_HANDLE_VALUE ) {	// 検索に失敗
		fprintf( stderr, "error: %s matches no files\n", search_path );
		return 0;
	}
	// 数を数える
	num = 0;
	do {
		num++;
		ret = FindNextFile( hSearch, &fFind );
	} while ( ret == TRUE );
	FindClose( hSearch );
	// もう一度検索してファイル名を格納する
	hSearch = FindFirstFile( wpath, &fFind); /* 元画像ディレクトリの探索開始 */
	files = (char **)calloc( num, sizeof(char *) );
	for ( i = 0, count = 0; i < num; i++ ) {
		wcstombs_s( &convertedChars, fname, kMaxPathLen, fFind.cFileName, _TRUNCATE );	// とりあえず変換
		if ( convertedChars > 0 ) {	// 変換成功なら
			wlen = wcslen(fFind.cFileName);
			files[count] = (char *)calloc( strlen(dir) + wlen + 1, sizeof(char) );
			strcpy( files[count], dir );	// とりあえずディレクトリをコピー
			strcat( files[count], fname );	// ファイル名を結合
			count++;
		}
		else {	// 変換失敗
			fprintf( stderr, "warning: file name conversion failed\n" );
		}

		FindNextFile( hSearch, &fFind );
	}
	FindClose( hSearch );
	*files0 = files;
	return count;
#else
	ret = glob( search_path, 0, errfunc_fpath, &gt );	// globの開始
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
// パス文字列pathからディレクトリ部分と拡張子を除いた文字列を取得し，basenameに格納する
{
	int i, last_slash, first_dot;
	
	basename[0] = '\0';
	for ( i = 0, last_slash = -1, first_dot = -1; i < len && path[i] != '\0'; i++ ) {
		if ( path[i] == kDelimiterChar )	last_slash = i;
		if ( path[i] == '.'/*&& first_dot < 0*/ )	first_dot = i;
	}
	// lenまで終端文字が見つからなかった場合
	if ( i == len )	return 0;
	// ドットがなかった場合
	last_slash++;
	if ( first_dot < 0 )	first_dot = i - 1;
	strcpy( basename, path + last_slash );
	basename[first_dot - last_slash] = '\0';
	
	return 1;
}

int IsDat( char *str )
// strの末尾が.datか調べる
{
	int i, last_dot = -1;
	
	for ( i = 0; str[i] != '\0'; i++ ) {
		if ( str[i] == '.' )	last_dot = i;
	}
	if ( last_dot < 0 )	return 0;
	if ( !strcmp( str+last_dot+1, "dat" ) || !strcmp( str+last_dot+1, "DAT" ) )	return 1;
	else	return 0;
}
