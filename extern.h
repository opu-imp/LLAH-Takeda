/* �O���[�o���ϐ���1�ӏ��ɏW�߂邽�߂̒�` */
#ifdef	GLOBAL_DEFINE 
	#define	Extern    /*define*/
#else
	#define	Extern    extern
#endif

Extern int eEntireMode;		// ���[�h
Extern int eGroup1Num;			// n
Extern int eGroup2Num;			// m
Extern int eGroup3Num;			// f
Extern int eNumCom1;			// nCm
Extern int eNumCom2;			// mCf
Extern int eDiscNum;			// ���U�����x��
Extern int eDiscRes;			
Extern int eUseArea;			// �ʐϔ������
Extern int eRmRedund;			// �����ʑI��
Extern int eThinPs;			// �����_�T���v�����O
Extern int eCompressHash;		// ���X�g���k
Extern int eDocBit;			// ����ID�̃r�b�g��
Extern int ePointBit;			// �����_ID�̃r�b�g��
Extern int eQuotBit;			// ���̃r�b�g��
Extern int eHComp2DatByte;		// ���k���X�g�T�C�Y
Extern int eFByte;				// ���k�����̃��X�g�T�C�Y
Extern int eMaxHashCollision;	// �Փˉ񐔂̐����l
Extern int eDocNumForMakeDisc;	// ���U���t�@�C���쐬�ɗp���镶���摜��
Extern int ePropMakeNum;		// ���萔�v�Z�ɗp���镶���摜��
Extern int eThinNum;			// �T���v�����O��
Extern int eExperimentMode;	// �������[�h
Extern int eDirectHashCombine;	// �n�b�V�������������[�h
Extern int eLoadDiscHash;		// ���U���t�@�C���ǂݍ��݃n�b�V���\�z
Extern int eRetMultiVec;		// �}���`�v������
Extern int eMultiVecNum;		// �}���`�v�������ɂ�����x�N�g����
Extern int eDbDocs;			// �o�^�����摜��
Extern int eBounds;				// �}���`�v�������ł͈͎̔w��
Extern int eLoadDiscData;		// ���U���t�@�C�������[�h���ăn�b�V���\�z
Extern int eTrueHist;		// �^�q�X�g�O����
Extern int eReHash;			// �^�q�X�g�O����
Extern int eThresh;			// �^�q�X�g�O����
Extern int eBlockSize;		// ���萔
Extern int eCombineStart;
Extern int eCombineEnd;
Extern int eFeatureNum;
Extern int eSampling;
Extern int eMultiNum;

Extern int **com1;
Extern int **com2;

Extern double eDiscMin;
Extern double eDiscMax;
Extern double eProp;		// ���萔
Extern double eC;		// ���萔
Extern int eMask;

Extern char eSrcDir[kMaxPathLen];
Extern char eQueDir[kMaxPathLen];
Extern char eDataDir[kMaxPathLen];
Extern char eHashFileName[kMaxPathLen];
Extern char ePointFileName[kMaxPathLen];
Extern char eDiscFileName[kMaxPathLen];
Extern char eConfigFileName[kMaxPathLen];
Extern char eThinFileName[kMaxPathLen];
Extern char eSrcSuffix[kMaxPathLen];
Extern char ePFPrefix[kMaxPathLen];	// �n�b�V���̓_�t�@�C���̃f�B���N�g��
Extern char eCombineParentDir[kMaxPathLen];

Extern unsigned long long int eHashSize;		// �n�b�V���e�[�u���̑傫��
Extern unsigned long long int eHashStorageNum;

// ini�t�@�C���֌W
Extern int eTCPPort;	// TCP�ʐM�|�[�g
Extern int eProtocol;	// �ʐM�v���g�R��
Extern int ePointPort;	// �����_�|�[�g�iUDP�j
Extern int eResultPort;	// �������ʃ|�[�g�iUDP�j
Extern char eClientName[16];	// �N���C�A���g�̃}�V����
Extern char eServerName[16];	// �T�[�o�̃}�V����
