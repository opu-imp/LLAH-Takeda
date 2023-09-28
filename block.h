void StartWriteBlock( char *fname, int bsize );
void WriteBlock( unsigned char *dat, int size );
void FinishWriteBlock( void );
int StartReadBlock( char *fname, int bsize );
int ReadBlock( unsigned char *dat, int size );
void FinishReadBlock( void );
void WriteBlock2( unsigned char *dat, long long int size );
int ReadBlock2( unsigned char *dat, long long int size );
