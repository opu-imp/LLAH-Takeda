int Retrieve( CvPoint *ps, double *areas, int num, int *score, strDisc *disc, int *reg_nums, int *ret_time, strHash **hash );
int RetrieveCor( CvPoint *ps, double *areas, int num, int *score, int pcor[][2], int *pcornum0, strDisc *disc, int *reg_nums, strHash **hash );
int IsSucceed( char *str1, char *str2 );
double Calc12Diff( int *score );
