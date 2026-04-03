typedef struct _strDiv {	// �����̈�
	int xindex;	// x���̃C���f�b�N�X
	int yindex;	// y���̃C���f�b�N�X
	double dist;	// ���ڂ��Ă�������_�Ƃ̍ŏ�����
} strDiv;

typedef struct _strPointDist {	// �\�[�g�̂��߂̓����_�Ƌ����̍\����
	int pindex;	// �_�̃C���f�b�N�X
	double	dist;	// �_�܂ł̋���
} strPointDist;

void SecureNears( void );
void ClearNears( int num );
void ReleaseNears( void );
void MakeNearsFromCentres( cv::Point *ps, int num );
int SearchMinAreas( cv::Point *ps, double *areas, int num, int *reg_thinids );
void MakeNearsFromCentresDiv( cv::Point *ps, int pnum, cv::Size *size, int tx, int ty, int kn );
int SearchMinNears( cv::Point *ps, double *areas, int num, int *reg_thinids );
