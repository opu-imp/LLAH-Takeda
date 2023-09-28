long double GetPointsAngle(CvPoint p1, CvPoint p2);
void CalcOrderCWN( int pt, CvPoint *ps, int idx[], int num );
int FindStartPoint( char *inv_array, int st_num );
double CalcArea( CvPoint p1, CvPoint p2, CvPoint p3 );
double CalcAffineInv( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4 );
void CalcHindexAreaRatio( int *idxcom1, double *areas, char *hindex_area );
void CalcAffineAndAddHist( CvPoint *ps, int num, strHist *hist );
void CalcAffineRotateOnceAndAddHash( CvPoint *ps, double *areas, int n, int num, strDisc *disc, strHash **hash, int *pids, int *collision );
void CalcAffineRotateOnce( CvPoint *ps, double *areas, int n, int num, strDisc *disc, strHash **hash, double ***features, char ***area_features );
void RankFeatureAndAddHash(double ***features, char ***area_features, int doc_id, int p_num, strDisc *disc, double *areas, strHash **hash, int tx, int ty, int ***div_ps, int **div_pnum, CvPoint *ps);
void RankFeatureAndAddHash2(double ***features, char ***area_features, int doc_id, int p_num, strDisc *disc, double *areas, strHash **hash, int tx, int ty, int ***div_ps, int **div_pnum, int *spare_fnums, char ***spare_features );
void RankFeatureAndAddHash3(double ***features, char ***area_features, int doc_id, int p_num, strDisc *disc, double *areas, strHash **hash, int tx, int ty, int ***div_ps, int **div_pnum);
void AdditionalAffineToHash( CvPoint *ps, double *areas, int n, int num, strDisc *disc, strHash **hash, int *pids, int *collision );
void CalcAffineRotateOnce2( CvPoint *ps, double *areas, int n, int num, strDisc *disc, strHash **hash, double ***features, char ***area_features, int *pids );