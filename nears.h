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
void MakeNearsFromCentres( CvPoint *ps, int num );
int SearchMinAreas( CvPoint *ps, double *areas, int num, int *reg_thinids );
void MakeNearsFromCentresDiv( CvPoint *ps, int pnum, CvSize *size, int tx, int ty, int kn );
int SearchMinNears( CvPoint *ps, double *areas, int num, int *reg_thinids );
