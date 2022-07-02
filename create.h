#ifndef CREATEH
#define CREATE

/*CONSTANTS */
#define MODE 0777

/*Professor Functions */
u_int32_t extract_special_int(char *where, int len);
u_int32_t insert_special_int(char *where, size_t size, int32_t val);

/*For creating archives*/
void archiveend(FILE *writeFile);
void traverseAndCreate(FILE* writeFile, char *path, int verbose, int strict);

/*For listing archives*/
void listarchive(FILE *archive, char *path, int verbose, int strict);

/*For extracting archives */
int tar_extract(FILE *tar, const char *path, const mode_t mode,
                                                int verbose, int strict);


/*For all*/
#define BLOCKSIZE 512
typedef struct header{
    /*HEADER VARIABLE DECLARATIONS*/
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag[1];
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char padding[12]; /*padding to fulfill 512 blocks*/

}header;

unsigned int oct2uint(char * oct, unsigned int size);
#endif
