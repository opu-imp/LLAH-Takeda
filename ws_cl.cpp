#include "def_general.h"

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include "ws_cl.h"

SOCKET InitWinSockClTCP( u_short port, char *server_name )
// WinSock�̃N���C�A���g��������
{
	SOCKET sock;
	WSADATA wsaData;
	struct sockaddr_in server;
	unsigned int **addrptr;
	
	// winsock2�̏�����
	if ( WSAStartup( MAKEWORD(2,0), &wsaData ) != 0 ) {
		fprintf( stderr, "Error : WSAStartup failed\n" );
		return INVALID_SOCKET;
	}
	// �\�P�b�g�̍쐬
	sock = socket( AF_INET, SOCK_STREAM, 0 );
	if ( sock == INVALID_SOCKET ) {
		fprintf( stderr, "Error : socket error(%d)\n", WSAGetLastError() );
		return INVALID_SOCKET;
	}
	// �ڑ���w��p�\���̂̏���
	server.sin_family = AF_INET;
	server.sin_port = htons( port );
	server.sin_addr.S_un.S_addr = inet_addr( server_name );
	if ( server.sin_addr.S_un.S_addr == 0xffffffff ) {	// inet_addr�����s
		struct hostent *host;
		host = gethostbyname( server_name );	// �z�X�g������A�h���X���擾
		if ( host == NULL ) {
			if ( WSAGetLastError() == WSAHOST_NOT_FOUND )
				fprintf( stderr, "Error : host %s not found\n", server_name );
			else	fprintf( stderr, "Error : gethostbyname\n" );
			return INVALID_SOCKET;
		}
		addrptr = (unsigned int **)host->h_addr_list;	// �A�h���X���X�g���擾
		while ( *addrptr != NULL ) {	// ���ׂẴA�h���X�ɂ���
			server.sin_addr.S_un.S_addr = *(*addrptr);
			if ( connect( sock, (struct sockaddr *)&server, sizeof(server) ) == 0 )	break;	// connect����
			addrptr++;	// ���̃A�h���X�Ŏ���
		}
		if ( *addrptr == NULL ) {	// connect�����ׂĎ��s
			fprintf( stderr, "Error : connect(%d)\n", WSAGetLastError() );
			return INVALID_SOCKET;
		}
	} else {	// inet_addr()������
		if ( connect( sock, (struct sockaddr *)&server, sizeof(server)) != 0 ) {
			fprintf( stderr, "Error : connect(%d)\n", WSAGetLastError() );
			return INVALID_SOCKET;
		}
	}
	return sock;
}

SOCKET InitWinSockSvTCP( u_short port )
// TCP/IP�ʐM�ɗp����\�P�b�g�̏�����
{
	SOCKET sock;
	WSADATA wsaData;
	struct hostent *sv_ip;
	struct sockaddr_in sv_addr;
	unsigned int **addrptr;
	BOOL yes = 1;
	
	if ( WSAStartup( MAKEWORD(2,0), &wsaData ) != 0 ) {
		return -1;
	}

	// �\�P�b�g�̍쐬
	sock = socket( AF_INET, SOCK_STREAM, 0 );
	if ( sock == INVALID_SOCKET ) {
		printf( "socket error : %d\n", WSAGetLastError() );
		return -1;
	}

	// �\�P�b�g�̐ݒ�
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons( port );
	sv_addr.sin_addr.S_un.S_addr = INADDR_ANY;

	//// TIME_WAIT��Ԃ̃|�[�g�����݂��Ă��Ă�bind�ł���悤��
	//setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes) );
	if ( bind( sock, (struct sockaddr *)&sv_addr, sizeof(sv_addr) ) != 0 ) {
		printf("bind : %d\n", WSAGetLastError());
		return -1;
	}

	// TCP�N���C�A���g����̐ڑ��v����҂Ă��Ԃɂ���
	if (listen(sock, 5) != 0) {
		printf( "listen error : %d\n", WSAGetLastError() );
		return 1;
	}

	return sock;
}

SOCKET AcceptWinSockSvTCP( SOCKET sock1 )
{
	SOCKET sock2;
	int cl_size;
	struct sockaddr_in cl_addr;
	
	cl_size = sizeof( cl_addr );
	sock2 = accept( sock1, (struct sockaddr *)&cl_addr, &cl_size );
	
	return sock2;
}

SOCKET InitWinSockClUDP( u_short port, char *server_name, struct sockaddr_in *addr )
// WinSock�̃N���C�A���g���������iUDP�j
{
	SOCKET sock;
	WSADATA wsaData;
//	struct sockaddr_in addr;
	struct hostent *host;
	unsigned int **addrptr;

	WSAStartup(MAKEWORD(2,0), &wsaData);
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	host = gethostbyname( server_name );	// �z�X�g������A�h���X���擾
	addrptr = (unsigned int **)host->h_addr_list;	// �A�h���X���X�g���擾
	addr->sin_addr.S_un.S_addr = *(*addrptr);
	
	return sock;
}

SOCKET InitWinSockSvUDP( u_short port )
{
	int rtn;
	SOCKET sock;
	WSADATA wsaData;
	struct sockaddr_in addr;
	struct hostent *host;
	unsigned int **addrptr;

	WSAStartup(MAKEWORD(2,0), &wsaData);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	memset( &addr, 0, sizeof(addr) );
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;
//	addr.sin_addr.s_addr = INADDR_ANY;

//	host = gethostbyname( "kana" );	// �z�X�g������A�h���X���擾
//	addrptr = (unsigned int **)host->h_addr_list;	// �A�h���X���X�g���擾
//	addr.sin_addr.S_un.S_addr = *(*addrptr);


	rtn = bind( sock, (struct sockaddr *)&addr, sizeof(addr) );
//	printf("bind : %d\n", rtn);
	
	return sock;
}

void CloseWinSock( SOCKET sock )
// WinSock�̃N���C�A���g���I��
{
	closesocket( sock );
	WSACleanup();
}

void sockaddr_print( struct sockaddr *addrp, socklen_t addr_len )
// IP �A�h���X�ƃ|�[�g�ԍ���\��
{
	char host[kBuffSize];
	char port[kBuffSize];

	if( getnameinfo(addrp, addr_len, host, sizeof(host),port, sizeof(port), NI_NUMERICHOST|NI_NUMERICSERV)<0 )
		return;
	printf( "%s:%s", host, port );
}


void tcp_peeraddr_print( int com )
// �ʐM����(peer)�̃A�h���X(TCP/IP�̏ꍇ�AIP�A�h���X�ƃ|�[�g�ԍ�)��\��
{
	struct sockaddr_storage addr;
	socklen_t addr_len;
	addr_len = sizeof( addr );
	if( getpeername( com, (struct sockaddr *)&addr, &addr_len  )<0 )
	{
		perror("tcp_peeraddr_print");
		return;
	}
	printf("connection (fd==%d) from ", com );
	sockaddr_print( (struct sockaddr *)&addr, addr_len );
	printf("\n");
}