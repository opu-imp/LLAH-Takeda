int OutPutImage(cv::Mat img);
int SetPixU( cv::Mat &img, int x, int y, unsigned char *val );
int GetPixU( cv::Mat &img, int x, int y, unsigned char *val );
cv::Mat MakeFeaturePointImage( cv::Mat orig, cv::Mat connected, cv::Point *ps, int num );
void DrawCor( cv::Point *ps, int num, cv::Size img_size, int res, cv::Point *corps, int cornum, cv::Size corsize, int pcor[][2], int pcornum );
