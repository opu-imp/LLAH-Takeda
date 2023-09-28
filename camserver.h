#define	kTCP	(1)
#define	kUDP	(2)

#define	kDivX	4	/* MakeNearsFromCentresDiv用 */
#define	kDivY	5	/* MakeNearsFromCentresDiv用 */

#define	SEND_FLAG	0
#ifdef	WIN32
#define	RECV_FLAG	0
#else
#define	RECV_FLAG	MSG_WAITALL
#endif

#define	kMaxNameLen	(20)	/* クライアント名の最大長さ */
#define	kMaxDocNameLen	(20)	/* 画像名の最大長 */
#define	kMinPoints	(20)	/* USBカメラモードでの最小特徴点数 */
#define	kTopThr	3	/* 08/09/14 ラーオ語でrejectしすぎなので変更 */
#define	kMinPointsToCalcParam	(4)	/* 射影変換パラメータを計算する際の最小対応点数（4以上にすること）*/

#define	kRecvBuffSize	(sizeof(CvSize) + sizeof(int) + sizeof(CvPoint) * kMaxPointNum)	/* 受信時のバッファのサイズ */
#define	kRecvBuffSizeAreas	(sizeof(CvSize) + sizeof(int) + (sizeof(CvPoint) + sizeof(unsigned short)) * kMaxPointNum)	/* 受信時のバッファのサイズ（面積も受信バージョン） */
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
