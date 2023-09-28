unsigned long long int HashFunc( char *index, int num, unsigned char *quotient);
unsigned long long int HashFuncArea( char *index, char *index_area, int num, unsigned char *quotient);
void AddHash( unsigned int index, int doc, short point, unsigned char quotient, strHash **hash );
void AddHashCompress( unsigned long long int index, unsigned int doc, unsigned short point, unsigned char quotient, strHash **hash );
strHash **InitHash( void );
void RefineHash( strHash **hash, int *spare_fnums, char ***spare_features );
void RemoveListSameIndexAndQuotient( strHash **hash );
void CalculateAverageCollision (strHash **hash);

#ifndef WIN32
int CalcMemory( void );
#endif