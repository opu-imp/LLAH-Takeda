#define	kTCP	(1)
#define	kUDP	(2)

#define	kDivX	4	/* MakeNearsFromCentresDiv�p */
#define	kDivY	5	/* MakeNearsFromCentresDiv�p */

#define	SEND_FLAG	0
#ifdef	WIN32
#define	RECV_FLAG	0
#else
#define	RECV_FLAG	MSG_WAITALL
#endif

#define	kMaxNameLen	(20)	/* �N���C�A���g���̍ő咷�� */
#define	kMaxDocNameLen	(20)	/* �摜���̍ő咷 */
#define	kMinPoints	(20)	/* USB�J�������[�h�ł̍ŏ������_�� */
#define	kTopThr	3	/* 08/09/14 ���[�I���reject�������Ȃ̂ŕύX */
#define	kMinPointsToCalcParam	(4)	/* �ˉe�ϊ��p�����[�^���v�Z����ۂ̍ŏ��Ή��_���i4�ȏ�ɂ��邱�Ɓj*/

#define	kRecvBuffSize	(sizeof(CvSize) + sizeof(int) + sizeof(CvPoint) * kMaxPointNum)	/* ��M���̃o�b�t�@�̃T�C�Y */
#define	kRecvBuffSizeAreas	(sizeof(CvSize) + sizeof(int) + (sizeof(CvPoint) + sizeof(unsigned short)) * kMaxPointNum)	/* ��M���̃o�b�t�@�̃T�C�Y�i�ʐς���M�o�[�W�����j */
#define	kSendBuffSize	(kMaxDocNameLen + sizeof(strProjParam) + sizeof(CvSize))
#define	kSendBuffSize3D	(kMaxDocNameLen + sizeof(strProjParam) + sizeof(CvSize) + sizeof(float)*16)
#define kSendBuffSizeCor	(kMaxDocNameLen + sizeof(strProjParam) + sizeof(CvSize) + sizeof(int) + sizeof(CvPoint)*kMaxPointNum*2 )
int RecvComSetting( SOCKET sid, int *ptc, int *pt_port, int *res_port, char *cl_name );
void SendResultName( SOCKET sock, char *result_name );
int SendResultParam( SOCKET sid, char *doc_name, int len, strProjParam *param, CvSize *img_size, struct sockaddr_in *addr, int ptc );
int RecvPointsAreas( SOCKET sid, CvPoint **ps0, double **areas0, CvSize *size );
void RetrieveUSBCamServer( void );
int SendResultCor( SOCKET sid, char *doc_name, int len, strProjParam *param, CvSize size, int pcor[][2], int pcornum, CvPoint *ps, CvPoint *psall, struct sockaddr_in *addr, int ptc );
int RecvFlag( SOCKET sid );
int SendDoc( SOCKET sid, char *doc_name, struct sockaddr_in *addr, int ptc );
void ExtractRAndTFromP(strProjParam param, float *tranceParam1, float *tranceParam2);
void ExtractRAndTFromP2(strProjParam param, float *tranceParam);
int SendResultParam3D( SOCKET sid, char *doc_name, int len, strProjParam *param, float *tranceParam1, float *tranceParam2, CvSize *img_size, struct sockaddr_in *addr, int ptc );
void CalcRotationAndTranslation( CvPoint *ps, CvPoint *reg_ps, int corres[][2], int cor_num, float *tranceParam1 );
