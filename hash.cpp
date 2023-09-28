#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
//プロセス ID を得るため
#include <sys/types.h>
#include <unistd.h>
#endif

#include "def_general.h"
#include "extern.h"
#include "hcompress.h"

strHash cut; // カットされたハッシュはここにつなげる

#ifndef WIN32
int CalcMemory( void )
// 使用メモリを計測
{	
	// メモリ使用量(登録データ分)を調べる
	pid_t pid = getpid();
	FILE *fp;
	char command_str[1024];
	char membuf[1024];
	int memsize=0;
	
	sprintf(command_str, "ps up %d|awk '{print $5 }' | tail -1", pid);
	//   sprintf(command_str, "ps up %d|awk '{print $11 \"\\t\" $5 \"\\t\" $6 }' | tail -1", pid);
	if ((fp=popen(command_str,"r"))==NULL) {
		fprintf(stderr, "ps failed\n");
		fprintf(stderr, "%s\n", command_str);
		//     exit(1);
	}
	fgets(membuf, sizeof(membuf), fp);
	printf("#mem(reg):%s", membuf);
	pclose(fp);

	memsize = atoi(membuf);

	return memsize;
}
#endif

unsigned long long int HashFunc( char *index, int num, unsigned char *quotient)
// ハッシュ値を求めるときの商も求める
// 特徴ベクトルで比較せずに商で比較するため
{
	int i;
	unsigned long long int ret = 0;
	*quotient = 0;

	for ( i = 0; i < eNumCom2; i++ ) {
		ret *= num;
		ret += index[i];
		*quotient += (unsigned char)(ret / eHashSize);	// 商の計算
		ret %= eHashSize;
	}

	return ret;
}

unsigned long long int HashFuncArea( char *index, char *index_area, int num, unsigned char *quotient)
// ハッシュ値を求めるときの商も求める
// 特徴ベクトルで比較せずに商で比較するため
{
	int i;
	unsigned long long int ret = 0;
	*quotient = 0;

	for ( i = 0; i < eGroup2Num; i++ ) {
		ret *= eGroup2Num;
		ret += index_area[i];
		*quotient += (unsigned char)(ret / eHashSize);	// 商の計算
		ret %= eHashSize;
	}
	for ( i = 0; i < eNumCom2; i++ ) {
		ret *= num;
		ret += index[i];
		*quotient += (unsigned char)(ret / eHashSize);	// 商の計算
		ret %= eHashSize;
	}
	return ret;
}

strHash **InitHash( void )
// ハッシュを確保
{
	int i;
	strHash **hash = NULL;

	hash = ( strHash ** )malloc( sizeof( strHash * ) * eHashSize );
	if ( hash == NULL )	return NULL;
	for ( i = 0; i < eHashSize; i++ )	hash[i] = NULL;
	return hash;
}

void setHash( strHash *setPt, int doc, short point, unsigned char quotient )
// ハッシュに文書ID，特徴点ID，商を格納
{
	strHash *tmp_i;
	tmp_i = setPt;
	memcpy( tmp_i, &doc, sizeof( int ) );
	tmp_i = tmp_i + 4;
	memcpy( tmp_i, &point, sizeof( short ) );
	tmp_i = tmp_i + 2;
	memcpy( tmp_i, &quotient, sizeof( unsigned char ) );
}

void AddHash( unsigned int index, int doc, short point, unsigned char quotient, strHash **hash )
{
	strHash *tmphash = NULL;
	unsigned char cnt;	// 衝突回数
	int nCut = eMaxHashCollision;	// 衝突回数の制限値

	if( hash[index] == &cut ) {	// 衝突回数の制限値を超えている
		// 何もしない
	}
	else if( hash[index] == NULL ) {	// 何も登録されてなければ
		tmphash = ( strHash * )malloc( 1 + eFByte );	// 配列確保 
		if( tmphash == NULL ) {
			fprintf( stderr, "Out of memory.\n" );
			exit( 1 );
		}
		*tmphash = ( unsigned char ) 1;	
		setHash( ( tmphash + 1 ), doc, point, quotient );	// ハッシュにデータ格納
		hash[index] = tmphash;
	}
	else{
		cnt = *( hash[index] );		// 衝突回数の取得
		if( cnt >= nCut ) {			// 制限値を超えていたら
			free( hash[index] );	// 削除
			hash[index] = &cut;		// 削除したものはここにつなげる
		}else{
			tmphash = ( strHash * )realloc( hash[index], 1 + (cnt+1) * eFByte );	// 配列の再取得
			if( tmphash == NULL ) {
				fprintf( stderr, "Out of memory.\n" );
				exit( 1 );
			}
			setHash( ( tmphash + 1 + eFByte * (*tmphash) ), doc, point, quotient );	// ハッシュにデータ格納
			*tmphash = ( *tmphash ) + 1;	// 衝突回数の更新
			hash[index] = tmphash;
		}
	}
}


void setHashCompress( strHash *setPt, unsigned char *dat )
// ハッシュに文書ID，特徴点ID，商を格納
{
	strHash *tmp_i;
	tmp_i = setPt;
	memcpy( tmp_i, dat, sizeof(char)*eHComp2DatByte );
}

void AddHashCompress( unsigned long long int index, unsigned int doc, unsigned short point, unsigned char quotient, strHash **hash )
{
	strHash *tmphash = NULL;
	unsigned char *dat;
	unsigned char cnt;	// 衝突回数
	int nCut = eMaxHashCollision;	// 衝突回数の制限値

	if( hash[index] == &cut ) {	// 衝突回数の制限値を超えている
		// 何もしない
	}
	else if( hash[index] == NULL ) {	// 何も登録されてなければ
		tmphash = ( strHash * )malloc( 1 + eHComp2DatByte );	// 配列確保 
		if( tmphash == NULL ) {
			fprintf( stderr, "Out of memory.\n" );
			exit( 1 );
		}
		*tmphash = ( unsigned char ) 1;	
		dat = MakeHComp2Dat( doc, point, quotient );
		setHashCompress( ( tmphash + 1 ), dat );	// ハッシュにデータ格納
		hash[index] = tmphash;
		free( dat );
	}
	else{
		cnt = *( hash[index] );		// 衝突回数の取得
		if( cnt >= nCut ) {			// 制限値を超えていたら
			free( hash[index] );	// 削除
			hash[index] = &cut;		// 削除したものはここにつなげる
		}else{
			tmphash = ( strHash * )realloc( hash[index], 1 + (cnt+1) * eHComp2DatByte );	// 配列の再取得
			if( tmphash == NULL ) {
				fprintf( stderr, "Out of memory.\n" );
				exit( 1 );
			}
			
			dat = MakeHComp2Dat( doc, point, quotient );
			setHashCompress( ( tmphash + 1 + eHComp2DatByte * (*tmphash) ), dat );	// ハッシュにデータ格納
			
			*tmphash = ( *tmphash ) + 1;	// 衝突回数の更新
			hash[index] = tmphash;
			
			free( dat );
		}
	}
}

void SearchMaxCollisionHashIndex( unsigned int *index, int *num, strHash **hash)
{
	int count=0;
	int max = -1;
	unsigned int max_index;

	for ( unsigned int i = 0; i < eHashSize; i++ ) {
		if ( hash[i] != NULL && hash[i] != &cut ) {
			if( *hash[i] == 10 ) {
				count++;
			}
			if( max < *hash[i] ) {
				max = *hash[i];
				max_index = i;
			}
		}
	}
	printf("Max Col : %d\n", count);
	*index = max_index;
	*num = max;
}

void SearchMaxCollisionDocID( unsigned int *index, int *num, unsigned int *collision)
{
	int max = 0;
	unsigned int max_index;

	for( int i = 0; i < eDbDocs; i++ ) {
		if ( collision[i] > max ) {
			max = collision[i];
			max_index = i;
		}
	}

	*index = max_index;
	*num = max;
}

void RemoveListSameIndexAndQuotient( strHash **hash )
{
	unsigned char list_quo[254], quo;
	unsigned int list_doc[254], doc;
	unsigned short list_point[254], point;
	int *doc_list_num, *doc_list_num_after;
	int cp_list_num;
	int threash = 350;
	strHash *tmp_list;

	doc_list_num = (int *)calloc(eDbDocs, sizeof(int));
	for ( unsigned long long int i = 0; i < eHashSize; i++ ) {
		if ( hash[i] != NULL && hash[i] != &cut ) {
			for ( int j = 0; j < *hash[i]; j++ ) {
				ReadHComp2Dat( (hash[i]+1+eHComp2DatByte*j), &(list_doc[j]), &(list_point[j]), &(list_quo[j]) );
				doc_list_num[list_doc[j]]++;
			}
		}
	}


	int rm_flag = 0;
	for ( unsigned long long int i = 0; i < eHashSize; i++ ) {
	//for ( unsigned long long int i = 3349992247; i < 3349992248; i++ ) {
		if ( hash[i] != NULL && hash[i] != &cut ) {

			for ( int j = 0; j < *hash[i]-1; j++ ) {
				rm_flag = 0;
				for ( int k = j+1; k < *hash[i]; k++ ) {
					for ( int l = 0; l < *hash[i]; l++ ) {
						ReadHComp2Dat( (hash[i]+1+eHComp2DatByte*l), &(list_doc[l]), &(list_point[l]), &(list_quo[l]) );
					}
					if ( list_quo[j] == list_quo[k] ) {
						rm_flag = 1;
						if ( doc_list_num[list_doc[k]] < threash )
							continue;

						//for ( int l = 0; l < *hash[i]; l++ ) {
						//	printf("%u, %u, %u\n", list_doc[l], list_point[l], list_quo[l]);
						//}
						//printf("%u: %3d: %6u: %3d: %3u, %6u: %3d: %3u\n", i, *hash[i], list_doc[j], j, list_quo[j], list_doc[k], k, list_quo[k]);
						
						if ( k == *hash[i]-1 ) {
							strHash *tmphash = (strHash *)realloc( hash[i], 1 + (*hash[i]-1) * eHComp2DatByte );
							*tmphash -= 1;
							hash[i] = tmphash;
						} else {
							strHash *start_point = hash[i]+1+eHComp2DatByte*(k+1);
							cp_list_num = *hash[i] - k - 1;
							tmp_list = (strHash *)calloc( cp_list_num, eHComp2DatByte );
							memcpy( tmp_list, start_point, cp_list_num*eHComp2DatByte);
							
							strHash *tmphash = ( strHash * )realloc( hash[i], 1 + (*hash[i]-1) * eHComp2DatByte );
							memcpy(tmphash+1+eHComp2DatByte*k, tmp_list, cp_list_num*eHComp2DatByte);
							
							*tmphash -= 1;
							hash[i] = tmphash;
							free(tmp_list);
						}

						k--;
					}
				}
				if ( rm_flag == 1 ) {
					//puts("flag");
					if ( *hash[i] == 1 ) {
						//puts("remove");
						free( hash[i] );	// 削除
						hash[i] = &cut;		// 削除したものはここにつなげる
					} else {
						strHash *start_point = hash[i]+1+eHComp2DatByte*(j+1);
						int cp_list_num = *hash[i] - j - 1;
						tmp_list = (strHash *)calloc( cp_list_num, eHComp2DatByte );
						memcpy( tmp_list, start_point, cp_list_num*eHComp2DatByte);

						strHash *tmphash = ( strHash * )realloc( hash[i], 1 + (*hash[i]-1) * eHComp2DatByte );
						memcpy(tmphash+1+eHComp2DatByte*j, tmp_list, cp_list_num*eHComp2DatByte);
						*tmphash -= 1;
						hash[i] = tmphash;
					}
				}
			}
		}
	}
	
	doc_list_num_after = (int *)calloc(eDbDocs, sizeof(int));
	for ( unsigned long long int i = 0; i < eHashSize; i++ ) {
		if ( hash[i] != NULL && hash[i] != &cut ) {
			for ( int j = 0; j < *hash[i]; j++ ) {
				ReadHComp2Dat( (hash[i]+1+eHComp2DatByte*j), &(list_doc[j]), &(list_point[j]), &(list_quo[j]) );
				doc_list_num_after[list_doc[j]]++;
			}
		}
	}

	for ( int i = 0; i < 10000; i++ ) {
		if (  doc_list_num[i] != doc_list_num_after[i] )
		printf("%d: %5d, %5d\n", i, doc_list_num[i], doc_list_num_after[i]);
	}
}

//void RemoveListSameIndexAndQuotient( strHash **hash )
//{
//	unsigned char list_quo[254], quo;
//	unsigned int list_doc[254], doc;
//	unsigned short list_point[254], point;
//	int *doc_list_num, *doc_list_num_after;
//	strHash *tmp_list;
//	int threash = 350;
//
//	doc_list_num = (int *)calloc(10000, sizeof(int));
//	for ( unsigned long long int i = 0; i < eHashSize; i++ ) {
//		if ( hash[i] != NULL && hash[i] != &cut ) {
//			for ( int j = 0; j < *hash[i]; j++ ) {
//				ReadHComp2Dat( (hash[i]+1+eHComp2DatByte*j), &(list_doc[j]), &(list_point[j]), &(list_quo[j]) );
//				doc_list_num[list_doc[j]]++;
//			}
//		}
//	}
//
//
//	int rm_flag = 0;
//	for ( unsigned long long int i = 0; i < eHashSize; i++ ) {
//	//for ( unsigned long long int i = 3349992247; i < 3349992248; i++ ) {
//		if ( hash[i] != NULL && hash[i] != &cut ) {
//
//			for ( int j = 0; j < *hash[i]-1; j++ ) {
//				rm_flag = 0;
//				for ( int k = j+1; k < *hash[i]; k++ ) {
//					for ( int l = 0; l < *hash[i]; l++ ) {
//						ReadHComp2Dat( (hash[i]+1+eHComp2DatByte*l), &(list_doc[l]), &(list_point[l]), &(list_quo[l]) );
//					}
//					if ( list_quo[j] == list_quo[k] ) {
//						rm_flag = 1;
//
//						if ( doc_list_num[list_doc[k]] < threash )
//							continue;
//
//						//for ( int l = 0; l < *hash[i]; l++ ) {
//						//	printf("%u, %u, %u\n", list_doc[l], list_point[l], list_quo[l]);
//						//}
//						//printf("%u: %d: %u: %d: %u, %u: %d: %u\n", i, *hash[i], list_doc[j], j, list_quo[j], list_doc[k], k, list_quo[k]);
//						
//						if ( k == *hash[i]-1 ) {
//							strHash *tmphash = (strHash *)realloc( hash[i], 1 + (*hash[i]-1) * eHComp2DatByte );
//							*tmphash -= 1;
//							hash[i] = tmphash;
//						} else {
//							strHash *start_point = hash[i]+1+eHComp2DatByte*(k+1);
//							int cp_list_num = *hash[i] - k - 1;
//							tmp_list = (strHash *)calloc( cp_list_num, eHComp2DatByte );
//							memcpy( tmp_list, start_point, cp_list_num*eHComp2DatByte);
//							
//							strHash *tmphash = ( strHash * )realloc( hash[i], 1 + (*hash[i]-1) * eHComp2DatByte );
//							memcpy(tmphash+1+eHComp2DatByte*k, tmp_list, cp_list_num*eHComp2DatByte);
//							*tmphash -= 1;
//							hash[i] = tmphash;
//						}
//
//						k--;
//					}
//				}
//				if ( rm_flag == 1 ) {
//					//puts("flag");
//					if ( *hash[i] == 1 ) {
//						//puts("remove");
//						free( hash[i] );	// 削除
//						hash[i] = &cut;		// 削除したものはここにつなげる
//					} else {
//						strHash *start_point = hash[i]+1+eHComp2DatByte*(j+1);
//						int cp_list_num = *hash[i] - j - 1;
//						tmp_list = (strHash *)calloc( cp_list_num, eHComp2DatByte );
//						memcpy( tmp_list, start_point, cp_list_num*eHComp2DatByte);
//
//						strHash *tmphash = ( strHash * )realloc( hash[i], 1 + (*hash[i]-1) * eHComp2DatByte );
//						memcpy(tmphash+1+eHComp2DatByte*j, tmp_list, cp_list_num*eHComp2DatByte);
//						*tmphash -= 1;
//						hash[i] = tmphash;
//					}
//				}
//			}
//		}
//	}
//	
//	//doc_list_num_after = (int *)calloc(10000, sizeof(int));
//	//for ( unsigned long long int i = 0; i < eHashSize; i++ ) {
//	//	if ( hash[i] != NULL && hash[i] != &cut ) {
//	//		for ( int j = 0; j < *hash[i]; j++ ) {
//	//			ReadHComp2Dat( (hash[i]+1+eHComp2DatByte*j), &(list_doc[j]), &(list_point[j]), &(list_quo[j]) );
//	//			doc_list_num_after[list_doc[j]]++;
//	//		}
//	//	}
//	//}
//
//	//for ( int i = 0; i < 10000; i++ ) {
//	//	if (  doc_list_num[i] != doc_list_num_after[i] )
//	//	printf("%d: %5d, %5d\n", i, doc_list_num[i], doc_list_num_after[i]);
//	//}
//}

void RefineHash( strHash **hash, int *spare_fnums, char ***spare_features )
{
	char flag;
	unsigned int spare_index;
	int pid;
	unsigned char quotient;
	double collision_average_num = 9999.0;
	unsigned int *collision_doc = (unsigned int *)calloc(eDbDocs, sizeof(unsigned int));
	int max_collision_num = 9999;														// 衝突回数の最大値

	while ( max_collision_num > 5 ) {
		unsigned int max_collision_index;												// 最大値のインデックス
		SearchMaxCollisionHashIndex(&max_collision_index, &max_collision_num, hash);	// 最も衝突回数が多いインデックスの探索
		printf("Max Collision index : %u\n", max_collision_index);
		printf("Max Collision Num : %d\n", max_collision_num);
		int mcn = max_collision_num;

		// 衝突の中で最も多い文書IDを探索
		unsigned int reg_doc_id;		// 文書ID
		unsigned short reg_point_id;	// 特徴点ID
		unsigned char reg_quotient;		// 商
		int max_collision_doc_num = 0;															// 最も衝突回数の多い文書の衝突回数
		unsigned int max_collision_doc_id;														// 最も衝突回数の多い文書のID
		for( int i = 0; i < mcn; i++ ) {
			ReadHComp2Dat( (hash[max_collision_index] + 1 + eHComp2DatByte*i), &reg_doc_id, &reg_point_id, &reg_quotient );
			collision_doc[reg_doc_id]++;
		}
		SearchMaxCollisionDocID(&max_collision_doc_id, &max_collision_doc_num, collision_doc);	// 最も衝突回数の多い文書の探索
		printf("DOC_ID: %d\n", max_collision_doc_id);

		// リストの移動ポイントの取得
		int mvpt_num=0;
		int *mvpt = (int *)calloc(mcn, sizeof(int));
		for( int i = 0; i < mcn; i++ ) {
			ReadHComp2Dat( (hash[max_collision_index]+1+eHComp2DatByte*i), &reg_doc_id, &reg_point_id, &reg_quotient );
			if ( max_collision_doc_id == reg_doc_id )	mvpt[mvpt_num++] = i;
		}

		// リスト削除＋リスト移動＋新しいインデックスの登録
		for ( int i = max_collision_doc_num-1; i >= 0; i-- ) {
			//printf("%d, %d\n", mvpt[i], mcn);
			// 削除＋移動
			if ( mvpt[i] != mcn-1 ) {
				memcpy(hash[max_collision_index]+1+eHComp2DatByte*mvpt[i], hash[max_collision_index]+1+eHComp2DatByte*(mvpt[i]+1), sizeof(strHash)*(mcn-mvpt[i])*eHComp2DatByte);
				*hash[max_collision_index] -= 1;
				//mcn--;
				//strHash *tmphash = (strHash *)realloc( hash[max_collision_index], 1 + mcn*eHComp2DatByte );
				//*tmphash = ( *tmphash ) - 1;
				//hash[max_collision_index] = tmphash;
			} else {
				*hash[max_collision_index] -= 1;
				//mcn--;
				//strHash *tmphash = (strHash *)realloc( hash[max_collision_index], 1 + mcn*eHComp2DatByte );
				//*tmphash = ( *tmphash ) - 1;
				//hash[max_collision_index] = tmphash;
			}

			//// 登録
			//for ( int j = 0; j < spare_fnums[max_collision_doc_id]; j++ ) {
			//	flag	= *(char *)			(spare_features[max_collision_doc_id][j]);
			//	if ( flag == 1 ) 	continue;
			//	else {
			//		spare_index	= *(unsigned int *)	(spare_features[max_collision_doc_id][j]+sizeof(char));
			//		pid		= *(int *)			(spare_features[max_collision_doc_id][j]+sizeof(char)+sizeof(unsigned int));
			//		quotient= *(unsigned char *)(spare_features[max_collision_doc_id][j]+sizeof(char)+sizeof(unsigned int)+sizeof(int));
			//		int doc = *(int *)(spare_features[max_collision_doc_id][j]+sizeof(char)+sizeof(unsigned int)+sizeof(int)+sizeof(unsigned char));
			//		printf("%d, %d, %d, %10u, %4d, %4d\n", max_collision_doc_id, doc, flag, spare_index, pid, quotient);

			//		if ( eCompressHash )	AddHashCompress( spare_index, max_collision_doc_id, pid, quotient, hash );
			//		else 					AddHash( spare_index, max_collision_doc_id, pid, quotient, hash );

			//		*spare_features[max_collision_doc_id][j] = 1;
			//		break;
			//	}
			//}
		}

		//int count=0, all=0;
		//for ( int i = 0; i < eHashSize; i++ ) {
		//	if ( hash[i] != NULL && hash[i] != &cut ) {
		//		all++;
		//		count += *hash[i];
		//	}
		//}
		//collision_average_num = (double)count/(double)all;
		//printf("average collision: %lf\n", collision_average_num);
		free(mvpt);
		// 初期化
		for ( int i = 0; i < eDbDocs; i++ ) {
			if ( collision_doc[i] != 0 )	collision_doc[i] = 0;
		}

	}

	free(collision_doc);
	//	for ( int i = 0; i < *hash[22230480]; i++ ) {
	//	ReadHComp2Dat( (hash[22230480]+1+eHComp2DatByte*i), &reg_doc_id, &reg_point_id, &reg_quotient );
	//	printf("%u : %u : %u\n", reg_doc_id, reg_point_id, reg_quotient);
	//}
	//for ( int i = 0; i < *hash[10311429]; i++ ) {
	//	ReadHComp2Dat( (hash[10311429]+1+eHComp2DatByte*i), &reg_doc_id, &reg_point_id, &reg_quotient );
	//	printf("%u : %u : %u\n", reg_doc_id, reg_point_id, reg_quotient);
	//}
}

	//for( int i = 0; i < max_collision_num; i++ ) {
	//	ReadHComp2Dat( (hash[max_collision_index]+1+eHComp2DatByte*i), &reg_doc_id, &reg_point_id, &reg_quotient );
	//	printf("%u : %u : %u\n", reg_doc_id, reg_point_id, reg_quotient);
	//	//printf("%u\n", collision_doc[reg_doc_id]);
	//}

	//for ( int i = 0; i < *hash[22230480]; i++ ) {
	//	ReadHComp2Dat( (hash[22230480]+1+eHComp2DatByte*i), &reg_doc_id, &reg_point_id, &reg_quotient );
	//	printf("%u : %u : %u\n", reg_doc_id, reg_point_id, reg_quotient);
	//}
	//for ( int i = 0; i < *hash[10311429]; i++ ) {
	//	ReadHComp2Dat( (hash[10311429]+1+eHComp2DatByte*i), &reg_doc_id, &reg_point_id, &reg_quotient );
	//	printf("%u : %u : %u\n", reg_doc_id, reg_point_id, reg_quotient);
	//}

void CalculateAverageCollision (strHash **hash)
{
	unsigned long long int count=0, value=0;
	unsigned int hist[1024] = {0};

	for ( unsigned long long int ii = 0; ii < eHashSize; ii++ ) {
		if ( hash[ii] != NULL && hash[ii] != &cut) {
			//printf("%d\n", *hash[ii]);
			if ( *hash[ii] >= eMaxHashCollision ) {
				*hash[ii] = 0;
			}
			else {
				count++;
				value+=*hash[ii];
				hist[*hash[ii]]++;
			}
		}
	}
	printf("%llu, %llu, %lf\n", value, count, (double)value/(double)count);
	//for (int i = 0; i < 1024; i++ ) {
	//	printf("%d: %u\n", i, hist[i]);
	//}
}