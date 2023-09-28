int SetBits( unsigned char *dst, int dfrom, unsigned long src, int len );
unsigned long GetBits( unsigned char *src, int from, int len );
unsigned char *MakeHComp2Dat( unsigned long doc, unsigned long point, unsigned long quotient );
int ReadHComp2Dat( unsigned char *dat, unsigned int *pdoc, unsigned short *ppoint, unsigned char *pquotient );
