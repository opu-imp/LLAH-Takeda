#include <stdio.h>
#include <stdlib.h>

#include "def_general.h"
#include "extern.h"
#include "hcompress.h"

int SetBits( unsigned char *dst, int dfrom, unsigned long src, int len )
// �r�b�g�f�[�^�̃R�s�[
// src����unsigned long(8�o�C�g�j�Ƃ�������
// src�̉��ʂ���len�r�b�g��dst��dfrom����len�r�b�g�܂ŃR�s�[����
{
	int i, cp_len, done_len;
	unsigned char dmask, smask;
	
	if ( (dfrom + len) % 8 > len ) {	// 8�r�b�g�����ł܂����Ȃ��ꍇ
		i = (int)((dfrom + len) / 8);	// �ŏ��ɃR�s�[����ꏊ
		smask = ((0x0001 << len) - 1) << (8 - ((dfrom + len) % 8));	// src�̃}�X�N
		dmask = 0x00ff & ( ~smask );	// dst�̃}�X�N
		dst[i] = (dst[i] & dmask) | ((src << (8 - ((dfrom + len) % 8))) & smask);
		return 1;
	}
	// 8�r�b�g�ȏ�̏ꍇ
	// �E�[�̏���
	i = (int)((dfrom + len) / 8);	// �ŏ��ɃR�s�[����ꏊ
	cp_len = (dfrom + len) % 8;	// �r�b�g��
//	printf("%d, %d\n", i, cp_len);
	dmask = (0x0001 << (8 - cp_len)) - 1;	// dst�̃r�b�g�}�X�N
	smask = (0x0001 << cp_len) - 1;	// src�̃r�b�g�}�X�N
	dst[i] = ( dst[i] & dmask ) | ( ( src & smask ) << ( 8 - cp_len ) );
	src = src >> cp_len;	// src���V�t�g
	for ( done_len = cp_len; done_len < len; done_len += cp_len ) {
		i--;	// �Z�b�g����ꏊ�i�o�C�g�P�ʁj���ړ�
		cp_len = len - done_len;
		if ( cp_len > 8 )	cp_len = 8;	// �ő�8�r�b�g
//		printf("%d, %d\n", i, cp_len);
		dmask = 0x00ff & ~((0x0001 << cp_len) - 1);	// dst�̃r�b�g�}�X�N�i��ʁj
		smask = (0x0001 << cp_len) - 1;	// src�̃r�b�g�}�X�N
		dst[i] = ( dst[i] & dmask ) | ( src & smask );
		src = src >> cp_len;	// src���V�t�g
	}
	return 1;
}

unsigned long GetBits( unsigned char *src, int from, int len )
// �r�b�g�f�[�^�̎擾
{
	int i, cp_len, done_len;
	unsigned char mask;
	unsigned long res = 0;
	
	if ( (from + len) % 8 > len ) {	// 8�r�b�g�����ł܂����Ȃ��ꍇ
		i = (int)((from + len) / 8);	// �R�s�[����ꏊ
		mask = (0x0001 << len) -1;	// �}�X�N
		res = ( src[i] >> (8 - (from + len) % 8) ) & mask;	// �V�t�g���ă}�X�N��AND
		return res;
	}
	// 8�r�b�g�ȏ�������͂܂����ꍇ
	// ���[�̏���
	i = (int)(from / 8);	// �ŏ��ɃR�s�[����ꏊ
	cp_len = 8 - from % 8;	// �r�b�g��
//	printf("%d, %d\n", i, cp_len);
	mask = (0x0001 << cp_len) - 1;	// src�̃r�b�g�}�X�N
	res = src[i] & mask;	// �}�X�N��AND
	for ( done_len = cp_len; done_len < len; done_len += cp_len ) {
		i++;	// �Z�b�g����ꏊ�i�o�C�g�P�ʁj���ړ�
		cp_len = len - done_len;
		if ( cp_len > 8 )	cp_len = 8;	// �ő�8�r�b�g
//		printf("%d, %d\n", i, cp_len);
		mask = (0x0001 << cp_len) - 1;
		res = (res << cp_len) | ((src[i] >> (8 - cp_len)) & mask);
	}
	return res;
}

unsigned char *MakeHComp2Dat( unsigned long doc, unsigned long point, unsigned long quotient )
// �n�b�V�����X�g���쐬
// 07/07/05 �ʐϓ����ʂ̋L�^��ǉ�
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
// �n�b�V�����X�g�̓ǂݍ���
// 07/07/20 �ʐϓ����ʂ̓ǂݍ��݂�ǉ�
{
	//printf("%d:%d:%d\n", eDocBit, ePointBit, eQuotBit);
	*pdoc = (unsigned int)GetBits( dat, 0, eDocBit );
	*ppoint = (unsigned short)GetBits( dat, eDocBit, ePointBit );
	*pquotient = (unsigned char)GetBits( dat, eDocBit+ePointBit, eQuotBit );
	
	return 1;
}
