SOCKET InitWinSockClTCP( u_short port, char *server_name );
SOCKET InitWinSockClUDP( u_short port, char *server_name, struct sockaddr_in *addr );
void CloseWinSock( SOCKET sock );
SOCKET InitWinSockSvUDP( u_short port );
SOCKET InitWinSockSvTCP( u_short port );
SOCKET AcceptWinSockSvTCP( SOCKET sid1 );
void tcp_peeraddr_print( int com );
