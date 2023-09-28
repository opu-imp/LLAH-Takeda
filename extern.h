/* グローバル変数を1箇所に集めるための定義 */
#ifdef	GLOBAL_DEFINE 
	#define	Extern    /*define*/
#else
	#define	Extern    extern
#endif

Extern int eEntireMode;		// モード
Extern int eGroup1Num;			// n
Extern int eGroup2Num;			// m
Extern int eGroup3Num;			// f
Extern int eNumCom1;			// nCm
Extern int eNumCom2;			// mCf
Extern int eDiscNum;			// 離散化レベル
Extern int eDiscRes;			
Extern int eUseArea;			// 面積比特徴量
Extern int eRmRedund;			// 特徴量選択
Extern int eThinPs;			// 特徴点サンプリング
Extern int eCompressHash;		// リスト圧縮
Extern int eDocBit;			// 文書IDのビット数
Extern int ePointBit;			// 特徴点IDのビット数
Extern int eQuotBit;			// 商のビット数
Extern int eHComp2DatByte;		// 圧縮リストサイズ
Extern int eFByte;				// 圧縮無しのリストサイズ
Extern int eMaxHashCollision;	// 衝突回数の制限値
Extern int eDocNumForMakeDisc;	// 離散化ファイル作成に用いる文書画像数
Extern int ePropMakeNum;		// 比例定数計算に用いる文書画像数
Extern int eThinNum;			// サンプリング数
Extern int eExperimentMode;	// 実験モード
Extern int eDirectHashCombine;	// ハッシュ結合検索モード
Extern int eLoadDiscHash;		// 離散化ファイル読み込みハッシュ構築
Extern int eRetMultiVec;		// マルチプル検索
Extern int eMultiVecNum;		// マルチプル検索におけるベクトル数
Extern int eDbDocs;			// 登録文書画像数
Extern int eBounds;				// マルチプル検索での範囲指定
Extern int eLoadDiscData;		// 離散化ファイルをロードしてハッシュ構築
Extern int eTrueHist;		// 真ヒストグラム
Extern int eReHash;			// 真ヒストグラム
Extern int eThresh;			// 真ヒストグラム
Extern int eBlockSize;		// 比例定数
Extern int eCombineStart;
Extern int eCombineEnd;
Extern int eFeatureNum;
Extern int eSampling;
Extern int eMultiNum;

Extern int **com1;
Extern int **com2;

Extern double eDiscMin;
Extern double eDiscMax;
Extern double eProp;		// 比例定数
Extern double eC;		// 比例定数
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
Extern char ePFPrefix[kMaxPathLen];	// ハッシュの点ファイルのディレクトリ
Extern char eCombineParentDir[kMaxPathLen];

Extern unsigned long long int eHashSize;		// ハッシュテーブルの大きさ
Extern unsigned long long int eHashStorageNum;

// iniファイル関係
Extern int eTCPPort;	// TCP通信ポート
Extern int eProtocol;	// 通信プロトコル
Extern int ePointPort;	// 特徴点ポート（UDP）
Extern int eResultPort;	// 検索結果ポート（UDP）
Extern char eClientName[16];	// クライアントのマシン名
Extern char eServerName[16];	// サーバのマシン名
