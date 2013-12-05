
int ReadPDB(FILE *infile, char *whatchain, int *aatype, double *xx, double *yy, double *zz, int *ab, int *ae);
void WritePDB(FILE *outfile, int n, char *whatchain, int *aatype, double *xx, double *yy, double *zz);
int ReadPDB_All_CA(FILE *infile, int *aatype, double *xx, double *yy, double *zz);
int ReadPDB_All_CA_Chain(FILE *infile, int *aatype, double *xx, double *yy, double *zz, char *chainArray);



//for passing into molviewer
typedef struct mol_thread_struct_ds {
    char *pdb_file;
    char chain_one;
    char chain_two; 
    int status;
    int mode;
} mol_thread_struct;
