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
#include <utime.h>
#include <sys/types.h>
#include <fcntl.h>

/*#define DEBUG*/

#define REG '0'
#define REGALT '\0'
#define DIRECTORY '5'
#define LINK '2'

int extract_entry(FILE *tar, struct header *header, int verbose);

/*ALGORITHM
    *IF(PATH == NULL) EXTRACT WHOLE ARCHIVE
    *IF(PATH != NULL) ONLY EXTRACT SUBTREE
*/

/*copyFile, reads file contents into block, prints as a string to newly
created file*/

static void permissions(char *mode, char permissions[]){
    mode_t perm;
    char **end = NULL;
    perm = strtol(mode, end, 8);

    if(end != NULL){
        fprintf(stderr, "failed to decypher permissions\n");
    }

    permissions[1] = (perm & S_IRUSR) ? 'r' : '-';
    permissions[2] = (perm & S_IWUSR) ? 'w' : '-';
    permissions[3] = (perm & S_IXUSR) ? 'x' : '-';
    permissions[4] = (perm & S_IRGRP) ? 'r' : '-';
    permissions[5] = (perm & S_IWGRP) ? 'w' : '-';
    permissions[6] = (perm & S_IXGRP) ? 'x' : '-';
    permissions[7] = (perm & S_IROTH) ? 'r' : '-';
    permissions[8] = (perm & S_IWOTH) ? 'w' : '-';
    permissions[9] = (perm & S_IXOTH) ? 'x' : '-';
    permissions[10] = '\0';

}
/*By default tar does not restore files permissions */
/*create a parent directory as necessary... i.e a file being in the archive
does not require all of its parent directories to exist*/
FILE* createfile(char *path, struct header *header){
    char mode[11] = {'\0'};
    permissions(header->mode, mode);
    int fileDesc;
    char *needle = NULL;
    needle = strstr(mode, "x");
    
    if(path[0] == '.' && path[strlen(path)] != '/')
        path = path + 2;
    
    if(needle != NULL){
        if((fileDesc = open(path, O_WRONLY | O_TRUNC | O_CREAT,
                     S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP |S_IXGRP |
                     S_IROTH |S_IWOTH | S_IXOTH)) == -1){
            perror("file open failed");
            exit(EXIT_FAILURE);
        }
    }else{
        if((fileDesc = open(path, O_WRONLY | O_TRUNC | O_CREAT,
                     S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH |
                     S_IWOTH)) == -1){
            perror("file open failed");
            exit(EXIT_FAILURE);
        }
    }

    FILE *fp = fdopen(fileDesc, "w");

    /*
    FILE *fp;
    if( (fp = fopen(path, "w")) == NULL)
        perror("can't create file");
    
    if(needle != NULL)
        chmod(path, 0777); /*rwx permissions *
    else{
        chmod(path, 0555);
    }*/

    return fp;

}

void restoretime(char *filename, char* mtime){
    /*struct timespec st_mtim;
    st_mtim.tv_sec = oct2uint(mtime, 12);*/
    struct utimbuf timebuff;
    timebuff.modtime = oct2uint(mtime, 12);
    utime(filename, &timebuff);
}


void fileName(char* name, char* prefix, char fileName[]){ 
    if(prefix != NULL){                                                         
        strcat(fileName, prefix);                                            
        strcat(fileName, "/");                                               
    }                                                                           
    strcat(fileName, name);                                                  
}       

unsigned int oct2uint(char * oct, unsigned int size){
    unsigned int out = 0;
    int i = 0;
    while ((i < size) && oct[i]){
        out = (out << 3) | (unsigned int) (oct[i++] - '0');
    }
    return out;
}

int headercheck(char* header_magic){
    if( header_magic[0] != 'u' || header_magic[1] != 's' || 
    header_magic[2] != 't' || header_magic[3] != 'a' || header_magic[4] != 'r')
        return -1;
    else
        return 0;

}


int tar_extract(FILE *tar, const char *path, const mode_t mode, 
                                                    int verbose, int strict){
    struct header header;
    struct header nullBlock;
    size_t offset = 0;    
    int size_read = 0;

    memset(&header, 0, sizeof(header));
    memset(&nullBlock, 0, sizeof(nullBlock));

        
    /*read a header to process */
    while(!feof(tar)){
        if((size_read = fread(header.name, 1, BLOCKSIZE, tar)) < 0)
            perror("failed in tar_extract to reach BLOCKSIZE");

        /*Check for end of archive */
        if(memcmp(&header, &nullBlock, sizeof(header)) == 0){
            offset = BLOCKSIZE;
            /*check for second NULL block to exit*/
            size_read = fread(header.name, 1, BLOCKSIZE, tar);
            if(size_read != BLOCKSIZE){
                fprintf(stderr, "Corrupt Header not 512 bytes");
                exit(EXIT_FAILURE);
             }
            /*if there is a second null block exit, other
            wise seek back to where you checked*/
            if(memcmp(&header, &nullBlock, sizeof(header)) == 0){
                return 0;
            }
            else{
                fseek(tar, -offset, SEEK_CUR);
            }

        }

        /*Do all my strict checks */
        /*Calculate checksum at the end as it is the sum of all the ASCII
        characters in the header*/
        char header_filename[PATH_MAX] = {'\0'}; /*convert header*/
        if(strlen(header.prefix) == 0){
            fileName(header.name, NULL, header_filename);
        }
        else{
            fileName(header.name, header.prefix, header_filename);
        }

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
                fprintf(stderr, "BAD HEADER WHEN EXTRACTING... EXITING\n");
                exit(EXIT_FAILURE);
            }
        }
        else if(!strict){
            if( (headercheck(header.magic)) != 0){
                fprintf(stderr, "BAD HEADER WHEN EXTRACTING... EXITING\n");
                exit(EXIT_FAILURE);
            }
        }
        
        /*extract entry */
        char *ret = NULL;
        if(path == NULL){
            extract_entry(tar, &header, verbose);
        }
        else if(path != NULL){
            if( (ret = strstr(header_filename, path)) != NULL)
                extract_entry(tar, &header, verbose);
            else{  
                size_t size = oct2uint(header.size, 11);
                /*MOVE FILE POINTER to next header */
                size_t offset = (size/ 512) * BLOCKSIZE;
                if( (size % BLOCKSIZE) > 0){
                    offset += BLOCKSIZE;
                }
                fseek(tar, offset, SEEK_CUR);
            }
        }


        /*RESET MEMORY BEFORE RELOOPING*/
        memset(&header, 0, sizeof(header));
        memset(&nullBlock, 0, sizeof(nullBlock));
    }
    
        
    return 0;
}

/*function that builds directory trees. doesn't require file in the archive to have existent 
parent directory */

int mkdir_recursive(const char *dir, const mode_t mode, int verbose){
    char temp[PATH_MAX];
    char *p = NULL;
    struct stat sb;
    int len;

    /*copy the path */
    len = strnlen(dir, PATH_MAX);
    if(len == 0 || len == PATH_MAX){ /*path too large */
        fprintf(stderr, "Path longer than 256 characters. \n");
        return -1;
    }

    memcpy(temp, dir, len);
    temp[len] = '\0';

    if(temp[len - 1] == '/'){
        temp[len - 1] = '\0';
    }

    /*check if path exists and is a directory */
    if(stat(temp, &sb) == 0){
        if(S_ISDIR(sb.st_mode)){
            return 0;
        }
    }
    
    /*recursively mkdir */
    for(p = temp + 1; *p; p++){
        if(*p == '/'){
            *p ='\0';

            /*test to see if path exists */
            if(stat(temp, &sb) != 0){
                if(mkdir(temp, mode) < 0){
                    perror("FAILED TO MAKE DIRECTORY IN recursive_mkdir\n");
                    return -1;
                }

            }

            *p = '/';
        }
        /*else if(!S_ISDIR(sb.st_mode)){
            return -1;
        }*/

    }
    

    if(stat(dir, &sb) == 0){
        if(S_ISDIR(sb.st_mode)){
            if (mkdir(dir, MODE) < 0){
                perror("FAILED TO MAKE DIRECTORY IN recursive_mkdir\n");
                return -1;
            }
        }
    }

    if(verbose){
        printf("%s\n", dir);
    }

    return 0;
}


/*FUNCTIONALITY 
    *CREATES AND COPIES FILES CONTENTS 
    *extract_entry handles the making of parent directories*/
static int copyfile(FILE *tar, struct header *header, int verbose){
    char path[PATH_MAX] = {'\0'};
    if(strlen(header->prefix) == 0){
        fileName(header->name, NULL, path);
    }
    else{
        fileName(header->name, header->prefix, path);
    }

    FILE *writefile = createfile(path, header); /*create file/restore mtime*/
    restoretime(path, header->mtime);
    if(writefile == NULL){
        fprintf(stderr, "error creating file in createfile()\n");
    }
    size_t size = oct2uint(header->size, 11);

    /*
    if(writefile == NULL){
        fprintf(stderr, "failed to create file \n");
        *MOVE FILE POINTER*
       size_t offset = (size/ 512) * BLOCKSIZE;
       if( (size % BLOCKSIZE) > 0){
            offset += BLOCKSIZE;
       }
       fseek(tar, offset, SEEK_CUR);
        return -1;
    }*/
    
    /*calculate how many blocks to read...this only works if caller
    sets file pointer to beginning of file*/
    int blocks = (size / BLOCKSIZE) * BLOCKSIZE;
    int remainderbytes = size % BLOCKSIZE;

    while(blocks--){
        char *buff[BLOCKSIZE];
        memset(&buff, 0, sizeof(buff));
        int size_read = 0;
        if((size_read = fread(buff, 1, BLOCKSIZE, tar)) > 0){
            fwrite(buff, 1, BLOCKSIZE, writefile);
        }
    }

    if(remainderbytes){
        char *buff[BLOCKSIZE];
        memset(&buff, 0, sizeof(buff));
        int size_read = 0;
        if((size_read = fread(buff, 1, BLOCKSIZE, tar)) > 0){
            fwrite(buff, 1, remainderbytes, writefile);
        }
    }
    
    fclose(writefile);
    if(verbose){
        printf("%s\n", path);
    }

    return 0;


}

int extract_entry(FILE *tar, struct header *header, int verbose){
    /*concatenate prefix and name to create path */ 
    
    char path[PATH_MAX] = {'\0'};
    if(strlen(header->prefix) == 0){
        fileName(header->name, NULL, path);
    }
    else{
        fileName(header->name, header->prefix, path);
    }

    if((header->typeflag[0] == REG || header->typeflag[0] == REGALT)){
        int len = 0;

        if(!(len = strlen(path))){
            fprintf(stderr, "EXTRACTION FAILED ON EMPTY NAME\n");
            return 0;
        }
        
        char temp[PATH_MAX] = {'\0'};
        strcpy(temp, path); 
        /*remove file from path to create directories recursively*/
        while(--len && (temp[len] != '/'))
            temp[len] = '\0';
        
        /*create necessary parent directories */
        if(mkdir_recursive(temp, MODE, verbose) != 0){
            fprintf(stderr, "Failed to make a directory");
            return -1;
        }
        
        /*create file and copy file contents */
        if(copyfile(tar, header, verbose) != 0){
            fprintf(stderr, "failed to copy contents\n");
        }
        

    }
    else if( (header->typeflag[0] == LINK) ){
        if(symlink(header->linkname, path) < 0){
            perror("Failed to create symlink\n");
        }
    }
    else if( (header->typeflag[0] == DIRECTORY) ){
        if( mkdir_recursive(path, MODE, verbose) < 0){
            fprintf(stderr, "Unable to make directory %s\n", path);
        }
        restoretime(path, header->mtime); /*restore mtime*/

    }
    
    return 0;
}





#ifdef DEBUG
int main(void){
    /* 
    mkdir_recursive("./testmake/tester/", MODE, 1);
    FILE* fp = createfile("./testmake/tester/file");
    char header_filename[PATH_MAX] = {'\0'};
    char* prefix = "tester";
    char *name = "home/";
    fileName(name, prefix, header_filename);
    printf("file name is %s \n", header_filename);
    char *mode = "0000775";
    mode_t modedec = oct2uint(mode, 7);
    printf("%s mode converted to decimal is %d\n", mode, modedec);
    */

    /*
    struct header header;
    struct header nullBlock;
    size_t offset = 0;    
    int size_read = 0;
    FILE *tar = fopen("./extract_entry_test", "r");
    memset(&header, 0, sizeof(header));
    fread(header.name, 1, BLOCKSIZE, tar);
    /*extract single file test*
    extract_entry(tar, &header, 1); 

    /*extract directory test*
    tar = fopen("./extract_entry_test2", "r");
    memset(&header, 0, sizeof(header));
    fread(header.name, 1, BLOCKSIZE, tar);
    extract_entry(tar, &header, 1);
    */

    FILE *tar = fopen("./test", "r");
    tar_extract(tar, NULL , MODE, 1, 0);
    return 0;
    
}
#endif
