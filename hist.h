int InitHist( strHist *hist, int size, double min, double max );
int Add2Hist( strHist *hist, double cr );
int Add2Hist2( strHist *hist, double cr );
void ReleaseHist( strHist *hist );
double GetMaxBin( strHist *hist );
