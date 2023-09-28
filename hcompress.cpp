#include <stdio.h>
#include <stdlib.h>

#include "def_general.h"
#include "extern.h"
#include "hcompress.h"

int SetBits( unsigned char *dst, int dfrom, unsigned long src, int len )
// ビットデータのコピー
// src側をunsigned long(8バイト）としたもの
// srcの下位からlenビットをdstのdfromからlenビットまでコピーする
{
	int i, cp_len, done_len;
	unsigned char dmask, smask;
	
	if ( (dfrom + len) % 8 > len ) {	// 8ビット未満でまたがない場合
		i = (int)((dfrom + len) / 8);	// 最初にコピーする場所
		smask = ((0x0001 << len) - 1) << (8 - ((dfrom + len) % 8));	// srcのマスク
		dmask = 0x00ff & ( ~smask );	// dstのマスク
		dst[i] = (dst[i] & dmask) | ((src << (8 - ((dfrom + len) % 8))) & smask);
		return 1;
	}
	// 8ビット以上の場合
	// 右端の処理
	i = (int)((dfrom + len) / 8);	// 最初にコピーする場所
	cp_len = (dfrom + len) % 8;	// ビット数
//	printf("%d, %d\n", i, cp_len);
	dmask = (0x0001 << (8 - cp_len)) - 1;	// dstのビットマスク
	smask = (0x0001 << cp_len) - 1;	// srcのビットマスク
	dst[i] = ( dst[i] & dmask ) | ( ( src & smask ) << ( 8 - cp_len ) );
	src = src >> cp_len;	// srcをシフト
	for ( done_len = cp_len; done_len < len; done_len += cp_len ) {
		i--;	// セットする場所（バイト単位）を移動
		cp_len = len - done_len;
		if ( cp_len > 8 )	cp_len = 8;	// 最大8ビット
//		printf("%d, %d\n", i, cp_len);
		dmask = 0x00ff & ~((0x0001 << cp_len) - 1);	// dstのビットマスク（上位）
		smask = (0x0001 << cp_len) - 1;	// srcのビットマスク
		dst[i] = ( dst[i] & dmask ) | ( src & smask );
		src = src >> cp_len;	// srcをシフト
	}
	return 1;
}

unsigned long GetBits( unsigned char *src, int from, int len )
// ビットデータの取得
{
	int i, cp_len, done_len;
	unsigned char mask;
	unsigned long res = 0;
	
	if ( (from + len) % 8 > len ) {	// 8ビット未満でまたがない場合
		i = (int)((from + len) / 8);	// コピーする場所
		mask = (0x0001 << len) -1;	// マスク
		res = ( src[i] >> (8 - (from + len) % 8) ) & mask;	// シフトしてマスクとAND
		return res;
	}
	// 8ビット以上もしくはまたぐ場合
	// 左端の処理
	i = (int)(from / 8);	// 最初にコピーする場所
	cp_len = 8 - from % 8;	// ビット数
//	printf("%d, %d\n", i, cp_len);
	mask = (0x0001 << cp_len) - 1;	// srcのビットマスク
	res = src[i] & mask;	// マスクとAND
	for ( done_len = cp_len; done_len < len; done_len += cp_len ) {
		i++;	// セットする場所（バイト単位）を移動
		cp_len = len - done_len;
		if ( cp_len > 8 )	cp_len = 8;	// 最大8ビット
//		printf("%d, %d\n", i, cp_len);
		mask = (0x0001 << cp_len) - 1;
		res = (res << cp_len) | ((src[i] >> (8 - cp_len)) & mask);
	}
	return res;
}

unsigned char *MakeHComp2Dat( unsigned long doc, unsigned long point, unsigned long quotient )
// ハッシュリストを作成
// 07/07/05 面積特徴量の記録を追加
{
	unsigned char *dat;
	//printf("%d:%d:%d\n", eDocBit, ePointBit, eQuotBit);
	dat = (unsigned char *)calloc( eHComp2DatByte, sizeof(unsigned char) );
	SetBits( dat, 0, doc, eDocBit );
	SetBits( dat, eDocBit, point, ePointBit );
	SetBits( dat, eDocBit + ePointBit, quotient, eQuotBit );

	return dat;
}

int ReadHComp2Dat( unsigned char *dat, unsigned int *pdoc, unsigned short *ppoint, unsigned char *pquotient )
// ハッシュリストの読み込み
// 07/07/20 面積特徴量の読み込みを追加
{
	//printf("%d:%d:%d\n", eDocBit, ePointBit, eQuotBit);
	*pdoc = (unsigned int)GetBits( dat, 0, eDocBit );
	*ppoint = (unsigned short)GetBits( dat, eDocBit, ePointBit );
	*pquotient = (unsigned char)GetBits( dat, eDocBit+ePointBit, eQuotBit );
	
	return 1;
}
