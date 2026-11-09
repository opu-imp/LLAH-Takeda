typedef struct _strDiv {	// 分割領域
	int xindex;	// x軸のインデックス
	int yindex;	// y軸のインデックス
	double dist;	// 注目している特徴点との最小距離
} strDiv;

typedef struct _strPointDist {	// ソートのための特徴点と距離の構造体
	int pindex;	// 点のインデックス
	double	dist;	// 点までの距離
} strPointDist;

void SecureNears( void );
void ClearNears( int num );
void ReleaseNears( void );
void MakeNearsFromCentres( cv::Point *ps, int num );
int SearchMinAreas( cv::Point *ps, double *areas, int num, int *reg_thinids );
void MakeNearsFromCentresDiv( cv::Point *ps, int pnum, cv::Size *size, int tx, int ty, int kn );
int SearchMinNears( cv::Point *ps, double *areas, int num, int *reg_thinids );
