#include<stdio.h>
#include<string.h>

int AATypeLookup(char *r)	// according to MJ96 matrix order
{
	int a = -1;

	if (!strcmp(r, "CYS"))
		a = 0;
	if (!strcmp(r, "MET"))
		a = 1;
	if (!strcmp(r, "PHE"))
		a = 2;
	if (!strcmp(r, "ILE"))
		a = 3;
	if (!strcmp(r, "LEU"))
		a = 4;
	if (!strcmp(r, "VAL"))
		a = 5;
	if (!strcmp(r, "TRP"))
		a = 6;
	if (!strcmp(r, "TYR"))
		a = 7;
	if (!strcmp(r, "ALA"))
		a = 8;
	if (!strcmp(r, "GLY"))
		a = 9;
	if (!strcmp(r, "THR"))
		a = 10;
	if (!strcmp(r, "SER"))
		a = 11;
	if (!strcmp(r, "ASN"))
		a = 12;
	if (!strcmp(r, "GLN"))
		a = 13;
	if (!strcmp(r, "ASP"))
		a = 14;
	if (!strcmp(r, "GLU"))
		a = 15;
	if (!strcmp(r, "HIS"))
		a = 16;
	if (!strcmp(r, "ARG"))
		a = 17;
	if (!strcmp(r, "LYS"))
		a = 18;
	if (!strcmp(r, "PRO"))
		a = 19;

	if (a == -1) {
		printf("unknown amino acid type %s\n", r);	/*exit(1); */
	}

	return a;
}

/*
http://www.umass.edu/microbio/rasmol/pdb.htm
RTyp  Num  Atm Res Ch  ResN X       Y       Z      Occ  Temp   PDB   Line
ATOM    1  N   ASP L   1    4.060   7.307   5.186  1.00 51.58  1FDL  93
ATOM    2  CA  ASP L   1    4.042   7.776   6.553  1.00 48.05  1FDL  94

RTyp: Record Type
Num: Serial number of the atom.  Each atom has a unique serial number.
Atm: Atom name (IUPAC format).
Res: Residue name (IUPAC format).
Ch: Chain to which the atom belongs (in this case, L for light chain
    of an antibody).
    ResN: Residue sequence number.
    X, Y, Z: Cartesian coordinates specifying atomic position in space.
    Occ: Occupancy factor
    Temp: Temperature factor (atoms disordered in the crystal have high
        temperature factors).
        PDB: The PDB data file unique identifier.
        Line: Line (record) number in the data file.
*/

int ReadPDB(FILE * infile, char *whatchain, int *aatype, double *xx, double *yy,
	    double *zz, int *act_beg, int *act_end)
{
	char s[100];
	char first4[5];
	char tmp[100];
	int resnum[10000];
// PDB file ATOM line fields
	char Atm[10], ResName[10], Ch[10];
//char RTyp[10], Atm[10], ResName[10], Ch[10], PDB[10];
	int ResNfirst, ResN;
//int ResNfirst, Num, ResN, Line, ii;
	double x, y, z;
//double x,y,z, Occ, Temp;
	int i = 0, firstresidue = 1;
	int j;

	while (!feof(infile)) {
		if (feof(infile))
			break;
		fgets(s, 90, infile);
		if (feof(infile))
			break;
//printf("%s",s);
		strncpy(first4, s, 4);
		first4[4] = 0;
//printf("%s\n",first4);

// some weird files have heteroatoms out of sequence and after TER (1g61)
		if ((!strcmp(first4, "TER ")) && (s[21] == whatchain[0])) {
			break;
		}

		if (!strcmp(first4, "ATOM")) {
			s[strlen(s) - 1] = 0;
//printf("--- %s ---\n",s); // need to change to position-based reading

// Atom type, C-alpha -> " CA "
			Atm[0] = s[12];
			Atm[1] = s[13];
			Atm[2] = s[14];
			Atm[3] = s[15];
			Atm[4] = 0;

// residue name, 3-letter caps
			ResName[0] = s[17];
			ResName[1] = s[18];
			ResName[2] = s[19];
			ResName[3] = 0;

			Ch[0] = s[21];
			Ch[1] = 0;	// chain ID
			if (Ch[0] == ' ')
				Ch[0] = '_';

// residue sequence number
			strncpy(tmp, s + 22, 4);
			tmp[5] = 0;
			sscanf(tmp, "%d", &ResN);

// weird: 1v05 starts: -3 -2 -1 2633 2634 ...
			if (ResN < 0)
				continue;
// weird: 3fzy starts: -8 ... -1 0 3428 ...
			if (ResN == 0)
				continue;

// x,y,z
			strncpy(tmp, s + 30, 24);
			tmp[25] = 0;
			sscanf(tmp, "%lf %lf %lf", &x, &y, &z);

			if ((!strcmp(Atm, " CA ")) && (!strcmp(Ch, whatchain))) {

				if ((firstresidue) && (ResN != 1)) {	/*fprintf(stderr,"weird pdb file: first residue not 1\n"); */
					ResNfirst = ResN;
				}

				if (firstresidue) {
					*act_beg = ResN;
				}

				resnum[i] = ResN - ResNfirst + 1;

//j = i; // sequential numbering
				j = ResN;	// numbering as in the file

				if ((!firstresidue) && (resnum[i] != resnum[i - 1] + 1)) {	/*fprintf(stderr,"weird pdb file: res %d after %d\n",resnum[i],resnum[i-1]); *//*exit(1); */
				}

				xx[j] = x;
				yy[j] = y;
				zz[j] = z;
				aatype[j] = AATypeLookup(ResName);
				if (aatype[j] == -1) {
					fprintf(stderr, "ReadPDB failed\n");
					return -1;
				}

				if (firstresidue)
					firstresidue = 0;
				i++;
			}

		}

	}

	*act_end = ResN;

	return i;
}

/*
reads coordinates and residue types of all CA atoms in a PDB file
all chains are lumped together
returns: # of atoms read
*/

#define PDB_LOG 0

int ReadPDB_All_CA(FILE * infile, int *aatype, double *xx, double *yy,
		   double *zz)
{
	char s[100];
	char first4[5];
	char tmp[100];
// PDB file ATOM line fields
	char Atm[10], ResName[10], Ch[10];
	int ResN;
	double x, y, z;
//char RTyp[10], Atm[10], ResName[10], Ch[10], PDB[10];
//int ResNfirst, Num, ResN, Line, ii;
//double x,y,z, Occ, Temp;

	int atomcount = 0;

	while (!feof(infile)) {
		if (feof(infile))
			break;
		fgets(s, 90, infile);
		if (feof(infile))
			break;
		strncpy(first4, s, 4);
		first4[4] = 0;

// some weird files have heteroatoms out of sequence and after TER (1g61)
//if ( (!strcmp(first4,"TER "))&&( s[21]==whatchain[0]) ) { break ; }

		if (!strcmp(first4, "ATOM")) {
			s[strlen(s) - 1] = 0;

// Atom type, C-alpha -> " CA "
			Atm[0] = s[12];
			Atm[1] = s[13];
			Atm[2] = s[14];
			Atm[3] = s[15];
			Atm[4] = 0;

// residue name, 3-letter caps
			ResName[0] = s[17];
			ResName[1] = s[18];
			ResName[2] = s[19];
			ResName[3] = 0;

			Ch[0] = s[21];
			Ch[1] = 0;	// chain ID
			if (Ch[0] == ' ')
				Ch[0] = '_';

// residue sequence number
			strncpy(tmp, s + 22, 4);
			tmp[5] = 0;
			sscanf(tmp, "%d", &ResN);

// weird: 1v05 starts: -3 -2 -1 2633 2634 ...
//if (ResN<0) continue;
// weird: 3fzy starts: -8 ... -1 0 3428 ...
//if (ResN==0) continue;

// x,y,z
			strncpy(tmp, s + 30, 24);
			tmp[25] = 0;
			sscanf(tmp, "%lf %lf %lf", &x, &y, &z);

			int ii = !strcmp(Atm, " CA ");
			int jj = !strcmp(Atm, " C  ");
			int kk = !strcmp(Atm, " N  ");
			if (ii) {
				xx[atomcount] = x;
				yy[atomcount] = y;
				zz[atomcount] = z;
				aatype[atomcount] = AATypeLookup(ResName);
				atomcount++;
			}
			if (PDB_LOG) {
				printf("1: %s", s);
				printf("2: first4: %s\n", first4);
				printf("3: Atm:_%s_\n", Atm);
				printf
				    ("4: CA val = %i, C value = %i, N value = %i\n\n",
				     ii, jj, kk);
			}

		}		//end if ATOM

	}			//end while 

	return atomcount;
}

int ReadPDB_All_CA_Chain(FILE * infile, int *aatype, double *xx, double *yy,
		   double *zz, char *chain)
{
	char s[100];
	char first4[5];
	char tmp[100];
// PDB file ATOM line fields
	char Atm[10], ResName[10], Ch[10];
	int ResN;
	double x, y, z;
//char RTyp[10], Atm[10], ResName[10], Ch[10], PDB[10];
//int ResNfirst, Num, ResN, Line, ii;
//double x,y,z, Occ, Temp;

	int atomcount = 0;

	while (!feof(infile)) {
		if (feof(infile))
			break;
		fgets(s, 90, infile);
		if (feof(infile))
			break;
		strncpy(first4, s, 4);
		first4[4] = 0;

// some weird files have heteroatoms out of sequence and after TER (1g61)
//if ( (!strcmp(first4,"TER "))&&( s[21]==whatchain[0]) ) { break ; }

		if (!strcmp(first4, "ATOM")) {
			s[strlen(s) - 1] = 0;

// Atom type, C-alpha -> " CA "
			Atm[0] = s[12];
			Atm[1] = s[13];
			Atm[2] = s[14];
			Atm[3] = s[15];
			Atm[4] = 0;

// residue name, 3-letter caps
			ResName[0] = s[17];
			ResName[1] = s[18];
			ResName[2] = s[19];
			ResName[3] = 0;

			Ch[0] = s[21];
			Ch[1] = 0;	// chain ID
			if (Ch[0] == ' ')
				Ch[0] = '_';

// residue sequence number
			strncpy(tmp, s + 22, 4);
			tmp[5] = 0;
			sscanf(tmp, "%d", &ResN);

// weird: 1v05 starts: -3 -2 -1 2633 2634 ...
//if (ResN<0) continue;
// weird: 3fzy starts: -8 ... -1 0 3428 ...
//if (ResN==0) continue;

// x,y,z
			strncpy(tmp, s + 30, 24);
			tmp[25] = 0;
			sscanf(tmp, "%lf %lf %lf", &x, &y, &z);

			int ii = !strcmp(Atm, " CA ");
			int jj = !strcmp(Atm, " C  ");
			int kk = !strcmp(Atm, " N  ");
			if (ii) {
				xx[atomcount] = x;
				yy[atomcount] = y;
				zz[atomcount] = z;
				aatype[atomcount] = AATypeLookup(ResName);
                chain[atomcount] = s[21];
				atomcount++;
			}
			if (PDB_LOG) {
				printf("1: %s", s);
				printf("2: first4: %s\n", first4);
				printf("3: Atm:_%s_\n", Atm);
				printf
				    ("4: CA val = %i, C value = %i, N value = %i\n\n",
				     ii, jj, kk);
			}

		}		//end if ATOM

	}			//end while 

	return atomcount;
}

void WritePDB(FILE * outfile, int n, char *whatchain, int *aatype, double *xx,
	      double *yy, double *zz)
{
	int i;

// positions and spaces ARE important!!!!!
//
//RTyp  Num  Atm Res Ch  ResN X       Y       Z      Occ  Temp   PDB   Line
//ATOM    1  N   ASP L   1    4.060   7.307   5.186  1.00 51.58  1FDL  93

	for (i = 0; i < n; i++)
		fprintf(outfile,
			"ATOM   %4d  CA  ALA %s %3d     %7.3f %7.3f %7.3f\n",
			i, whatchain, i, xx[i], yy[i], zz[i]);

}

/*
int main(void)
{
double xa[1000], ya[1000], za[1000];
int i,n,aatype[1000];
FILE *f;

n=ReadPDB(stdin,"_", aatype, xa,ya,za);
fprintf(stderr,"%d\n",n);

for(i=0;i<n;i++) if (aatype[i]<0) printf("error in aa type\n");

f =fopen("test.pdb","w");
WritePDB(f, n, "A", aatype, xa,ya,za);
fclose(f);

}
*/
