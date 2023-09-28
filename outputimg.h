int OutPutImage(IplImage *img);
int SetPixU( IplImage *img, int x, int y, unsigned char *val );
int GetPixU( IplImage *img, int x, int y, unsigned char *val );
IplImage *MakeFeaturePointImage( IplImage *orig, IplImage *connected, CvPoint *ps, int num );
void DrawCor( CvPoint *ps, int num, CvSize img_size, int res, CvPoint *corps, int cornum, CvSize corsize, int pcor[][2], int pcornum );
