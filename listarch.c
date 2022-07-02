#include <stdlib.h>                                                             
#include <stdio.h>                                                              
#include <string.h>                                                             
#include <sys/stat.h>                                                           
#include <dirent.h>                                                             
#include <sys/sysmacros.h>                                                      
#include <pwd.h>                                                                
#include <grp.h>                                                                
#include <unistd.h>                                                             
#include "create.h"
#include <time.h>
#include <assert.h>

/*#define DEBUG*/

typedef struct table{
    char permissions[11];
    char owners[70]; /*name owner/name group = 65*/
    int size;
    char mtime[50];
    char name[256]; /*max size of name is 255 + null*/


}table;

void permissions(char *mode, char typeflag, table *table);
void name(char *uname, char *gname, table *table);
void timetostr(char *mtime, table *table);
void name(char *uname, char *gname, table *table);
static void fileName(char* name, char* prefix, table *table);
void size(char *size, table *table);
void printTableVerbose(table *table);
void printTableNotVerbose(table *table);


static int headercheck(char* header_magic){
    if( header_magic[0] != 'u' || header_magic[1] != 's' ||
    header_magic[2] != 't' || header_magic[3] != 'a' || header_magic[4] != 'r')
        return -1;
    else
        return 0;

}


#ifdef DEBUG
int main(void){ 
    struct header header; 
    struct header nullBlock;
    memset(&header, 0, sizeof(header));
    memset(&nullBlock, 0, sizeof(nullBlock));
    table table;
    memset(&table, 0, sizeof(table));
    char **end = NULL;
    FILE* archive = fopen("./ref2", "r");
    int sizeRead;
    assert(memcmp(&header, &nullBlock, sizeof(header)) == 0);

    /*
    mode_t perm;
    sizeRead = fread(header.name, 1, BLOCKSIZE, archive);
    perm = strtol(header.mode, end, 8);
    printf("perm number is %d\n", perm);
    permissions(header.mode, header.typeflag[0], &table);
    printf("permissions are %s\n", table.permissions);
    name(header.uname, header.gname, &table);
    printf("name is %s\n", table.owners);
    size(header.size, &table);
    printf("size is %d\n", table.size);
    timetostr(header.mtime, &table);
    printf("time is %s\n", table.mtime);
    if(strlen(header.prefix) == 0)
        fileName(header.name, NULL, &table);
    printf("file name is %s\n", table.name);
    printTableVerbose(&table); */

    listarchive(archive, "tartest", 1, 1);
    return 0;
}
#endif

/*FUNCTIONALITY
    *ACCEPTS ARCHIVE FILE
    *LISTS CONTENTS OF ARCHIVE FILE IN ORDER, ONE PER LINE
    *IF NO NAMES ARE GIVEN ON THE COMMAND LINE, LISTS WHOLE ARCHIVE
    *IF PATH == NULL, MEANS NO NAME GIVEN
    *Accepts verbose flag...if 0 list non verbose, if 1 print verbose
*/

void listarchive(FILE *archive, char *path, int verbose, int strict){
    header header;
    struct header nullBlock; /*used to check if at the end of the archive */
    table table;
    int sizeRead = 0;
    size_t offset = 0; /*how much to offser file pointer*/
    
    /*initialize memory of table WE MUST DO THIS FOR EVERY ITERATION*/
    memset(&table, 0, sizeof(table));
    memset(&header, 0, sizeof(header));
    memset(&nullBlock, 0, sizeof(nullBlock));

    while(!feof(archive)){
       sizeRead = fread(header.name, 1, BLOCKSIZE, archive);
       if(sizeRead != BLOCKSIZE){
            fprintf(stderr, "Corrupt Header not 512 bytes");
            exit(EXIT_FAILURE);
       }
       
       /*IF BLOCK OF ALL NULL BYTES, SEEK TO NEXT ALL NULL TO 
       SEE IF END OF ARCHIVE*/
       if(memcmp(&header, &nullBlock, sizeof(header)) == 0){
            offset = BLOCKSIZE;
            /*check for second NULL block to exit*/
            sizeRead = fread(header.name, 1, BLOCKSIZE, archive);
            if(sizeRead != BLOCKSIZE){
                fprintf(stderr, "Corrupt Header not 512 bytes");
                exit(EXIT_FAILURE);
            }
            /*if there is a second null block exit, other
            wise seek back to where you checked*/
            if(memcmp(&header, &nullBlock, sizeof(header)) == 0){
                return;
            }
            else{
                fseek(archive, -offset, SEEK_CUR);
            }

       }

       /*CALCULATE LIST CONTENTS*/
       permissions(header.mode, header.typeflag[0], &table);
       name(header.uname, header.gname, &table);
       size(header.size, &table);
       timetostr(header.mtime, &table);
       if(strlen(header.prefix) == 0){
            fileName(header.name, NULL, &table);
       }
       else{  
            fileName(header.name, header.prefix, &table);
       }
        
        int strictfail = 0; /*flag to mark what not to list */
        size_t header_checksum = oct2uint(header.checksum, 7);
        memset(header.checksum, ' ', 8);
        size_t checksum = 0;
        const unsigned char *bytes = &header;
        for(int i = 0; i < sizeof(header); i++){
            checksum += bytes[i];
        }
        if(strict){
            if((checksum != header_checksum) || 
                (strcmp(header.magic,"ustar") 
                != 0) || (header.version[0] != '0' 
                                        && header.version[1] != '0')){
                fprintf(stderr, "DOES NOT FOLLOW STRICT STANDARD. WILL "
                "NOT BE LISTED\n");
                strictfail = 1;
            }
                int val;
                if( (val = extract_special_int(header.uid, 8)) != -1 ){
                    strictfail = 1;
                }
        
                if( (val = extract_special_int(header.gid, 8)) != -1){
                    strictfail = 1;
                }            
           }
        else if(!strict){
            if( (headercheck(header.magic)) != 0){
                fprintf(stderr, "DOES NOT FOLLOW STRICT STANDARD. WILL "
                "NOT BE LISTED\n");
                strictfail = 1;
            }
        }

       /*PRINT TABLE
            *strstr will check if the header name is present
            within the path argument
            *If no path given, prints all
            *if verbose flag is true, prints verbose
        */
       char *ret = NULL; /*defined for strstr*/
       if(path == NULL && strictfail == 0){
            if(verbose)
                printTableVerbose(&table);
            else
                printTableNotVerbose(&table);
       }
       else if( path != NULL && strictfail == 0 &&
                    ( (ret = strstr(table.name, path)) != NULL) ){
            if(verbose)
                printTableVerbose(&table);
            else
                printTableNotVerbose(&table);
       }

       /*MOVE FILE POINTER*/
       offset = (table.size / 512) * BLOCKSIZE;
       if( (table.size % BLOCKSIZE) > 0){
            offset += BLOCKSIZE;
       }
       fseek(archive, offset, SEEK_CUR);

       /*RESET MEMORY BEFORE RELOOPING*/ 
        memset(&table, 0, sizeof(table));
        memset(&header, 0, sizeof(header));
        memset(&nullBlock, 0, sizeof(nullBlock));
    }

    if(ferror(archive)){
        fprintf(stderr, "error reading header in listarchive");
    }
}


/*FUNCTIONALITY
    *CALCULATES THE PERMISSIONS AND PUTS IT IN TABLE STRUCT
*/
void permissions(char *mode, char typeflag, table *table){
    mode_t perm;
    char **end = NULL;
    perm = strtol(mode, end, 8);

    if(end != NULL){
        fprintf(stderr, "failed to decypher permissions\n");
    }

    if(typeflag == '2') /*symbolic link*/
        table->permissions[0] = 'l';
    else if(typeflag == '5') /*directory */
        table->permissions[0] = 'd';
    else{                   /*if any other type of file */
        table->permissions[0] = '-';
    }

    table->permissions[1] = (perm & S_IRUSR) ? 'r' : '-';
    table->permissions[2] = (perm & S_IWUSR) ? 'w' : '-';
    table->permissions[3] = (perm & S_IXUSR) ? 'x' : '-';
    table->permissions[4] = (perm & S_IRGRP) ? 'r' : '-';
    table->permissions[5] = (perm & S_IWGRP) ? 'w' : '-';
    table->permissions[6] = (perm & S_IXGRP) ? 'x' : '-';
    table->permissions[7] = (perm & S_IROTH) ? 'r' : '-';
    table->permissions[8] = (perm & S_IWOTH) ? 'w' : '-';
    table->permissions[9] = (perm & S_IXOTH) ? 'x' : '-';
    table->permissions[10] = '\0';

}

void name(char *uname, char *gname, table *table){
    strcat(table->owners, uname);
    strcat(table->owners, "/");
    strcat(table->owners, gname);
}

/*FUNCTIONALITY
    *Converts mtime to table of contents format
*/

void timetostr(char *mtime, table *table){
    char **end = NULL;
    struct tm* timeinfo;
    time_t mtimeconv = strtol(mtime, end, 8);
    if(end != NULL){
        fprintf(stderr, "Failed to convert mtime\n");
    }
    timeinfo = localtime(&mtimeconv);
    strftime(table->mtime, 50, "%Y-%m-%d %H:%M", timeinfo);
}

/*FUNCTIONALITY*
    *IF PREFIX IS NULL, PASS NULL TO PREFIX ARGUMENT
*/
static void fileName(char* name, char* prefix, table *table){
    if(prefix != NULL){
        strcat(table->name, prefix);
        strcat(table->name, "/");
    }
    strcat(table->name, name);
}

void size(char *size, table *table){
    char **end = NULL; 
    if(end != NULL){
        fprintf(stderr, "failed to decypher permissions\n");
    }
    else
        table->size = strtol(size, end, 8);
}

void printTableVerbose(table *table){
    printf("%-10s %-17s %8d %-16s %-s\n", table->permissions, table->owners,
            table->size, table->mtime, table->name);
}

void printTableNotVerbose(table *table){
    printf("%-s\n", table->name);
}
