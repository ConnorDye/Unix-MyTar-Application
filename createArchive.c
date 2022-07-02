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




int createheader(FILE *writeFile, char *path, int strict);
void copyfile(FILE *writeFile, FILE *input);
void tarfile(FILE* writeFile, char *path, int strict);

/*typedef struct header{
    *HEADER VARIABLE DECLARATIONS*
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
    char padding[12];
}header; */



/*CREATHEADER FUNCTIONALITY:
    *accepts a filetowrite to and the path of the optind argument from getopt
    *will lstat this file to get information for the header
    *write this lstat information to the file
    *path should be no longer than 255 bytes
*/
int createheader(FILE *writeFile, char *path, int strict){
    int prefix_flag = 0;
    struct group *g;
    struct passwd *u;
    struct stat sb;
    header header;
    memset(&header, 0, sizeof(header));
    
    if(lstat(path, &sb) < 0){
        perror("lstat failed in createHeader");
        exit(EXIT_FAILURE);
    }



    /*set contents of header*/
    if(strlen(path) + 1 <= 100){ /*no prefix */
        snprintf(header.name, 100, "%s", path); /*path + 2 to rmv the ./ */
        if(S_ISDIR(sb.st_mode)){
            snprintf(header.name, 100, "%s/", path); 
        }
    }
    else{
        prefix_flag = 1;        /*indicates prefix*/
        /*char *namePtr = path + 2;
        while((strlen(namePtr) + 1) > 100){
            namePtr++;
        }
        snprintf(header.name, 100, "%s", namePtr);
        *namePtr = '\0'; *path will now be prefix*/
        char *namePtr = path + strlen(path);
        namePtr = namePtr - 100;
        while(*namePtr != '/'){
            namePtr++;
        }
        snprintf(header.name, 100, "%s", namePtr);
        *namePtr = '\0'; /*path will now be prefix */
    }

    /*set flag*/
    if(S_ISREG(sb.st_mode)){
        header.typeflag[0] = '0';
    }
    else if(S_ISDIR(sb.st_mode)){
        header.typeflag[0] = '5';
    }
    else if (S_ISLNK(sb.st_mode)){
        header.typeflag[0] = '2';
        if((readlink(path, header.linkname, 100)) < 0)
            perror("readlink() error\n");
    }
    else{
        header.typeflag[0] = '\0';
    } 

    /*File mode*/
    snprintf(header.mode, 8, "%07o", sb.st_mode);
    header.mode[0] = '0'; /*removes type of file */
    header.mode[1] = '0';
    header.mode[2] = '0';

    /*CHECK IF OCTAL DIGIT IS GREATER THAN MAX SIZE */
    int size = 50; /*temporary variables*/
    char tempoctal[size];

    snprintf(tempoctal, 50, "%o", sb.st_uid);
    if( (strlen(tempoctal) > 7) && !strict){
        if((insert_special_int(header.uid, 8, sb.st_uid)) != 0){
            fprintf(stderr, 
                    "failed to convert octal to non conforming header\n");
            return 1; /*return 1 on failure*/
        }
    }
    else if( (strlen(tempoctal) > 7) && strict){
        fprintf(stderr, "Strict mode: octal size greater than maximum size\n");
    }
    else{
        snprintf(header.uid, 8, "%07o", sb.st_uid);
    }
    
    snprintf(tempoctal, 50, "%o", sb.st_gid);
    if( (strlen(tempoctal) > 7) && !strict){
        if((insert_special_int(header.gid, 8, sb.st_gid)) != 0){
            fprintf(stderr, 
                    "failed to convert octal to non conforming header\n");
            return 1; /*return 1 on failure*/
        }
    }
    else if( (strlen(tempoctal) > 7) && strict){
        fprintf(stderr, "Strict mode: octal size greater than maximum size\n");
    }
    else{
        snprintf(header.gid, 8, "%07o", sb.st_gid);
    }


    /*snprintf(header.gid, 8, "%07o", sb.st_gid);*/
    /*snprintf(header.uid, 8, "%07o", sb.st_uid); */
    
    if(S_ISDIR(sb.st_mode) || S_ISLNK(sb.st_mode)){
        long int size = 0;
        snprintf(header.size, 12, "%011lo", size);
    }else{
        snprintf(header.size, 12, "%011lo", sb.st_size);
    }

    snprintf(header.mtime, 12, "%011lo", sb.st_mtim.tv_sec);
    /*chksum calculated at the bottom */
    /*add typeflag at top */
    /*add linkname calculated in (S_ISLNK) loop */

    snprintf(header.magic, 6, "%s", "ustar");
    header.version[0] = '0'; /*snprintf attached NULL*/
    header.version[1] = '0';
    /*add uname */
    u = getpwuid(sb.st_uid);
    snprintf(header.uname, 32, "%s", u->pw_name);
    /*add gname */
    g = getgrgid(sb.st_gid);
    snprintf(header.gname, 32, "%s", g->gr_name);
    /*devmajor / minor printed as null */
    /*snprintf(header.devmajor, 8, "%07o", major(sb.st_dev));
    snprintf(header.devminor, 8, "%07o", minor(sb.st_dev));*/
    if(prefix_flag == 1){ /*prefix is null otherwise*/
        snprintf(header.prefix, 155, "%s", path);
    }

    
    /*Calculate checksum at the end as it is the sum of all the ASCII
    characters in the header*/
    memset(header.checksum, ' ', 8);
    size_t checksum = 0;
    const unsigned char *bytes = &header;
    for(int i = 0; i < sizeof(header); i++){
        checksum += bytes[i];
    }
    /*subtract checksum field and add back 8 spaces */
    snprintf(header.checksum, 8, "%07lo", checksum);
    
    /*write the header*/
    fwrite(bytes, 1, sizeof(header), writeFile);
    return 0; /*returns 0 on success */
}

/*FUNCTIONALITY:
    *ACCEPTS pointer to tar file and input file
    *writes in blocks of 512 bytes, until all contents of file is copied
    *if data read is less than 512 bytes, data is filled with nulls
    *accepts a end of archive flag to fill two all null blocks
*/
void copyfile(FILE *writeFile, FILE *input){
    unsigned char *buff[BLOCKSIZE];
    int sizeRead = 0;
    memset(&buff, 0, sizeof(buff));


    while((sizeRead = fread(buff, 1, BLOCKSIZE, input)) > 0){
        fwrite(buff, 1, BLOCKSIZE, writeFile);
        memset(&buff, 0, sizeof(buff));
    }

    if(ferror(input)){
        fprintf(stderr, "error reading file in copyfile()");
    }

}

void tarfile(FILE* writeFile, char *path, int strict){
    FILE *input; /*holds input file */ 
    struct stat sb;
    if((input = fopen(path, "r")) == NULL){
        fprintf(stderr, "Input file cannot be read");
        return;
    }
    if(lstat(path, &sb) == -1)
        fprintf(stderr, "stat error in tarfile()\n");

    /*if header is successfully written && not dir or link, copy contents*/
    if(createheader(writeFile, path, strict) == 0 && 
                        !S_ISDIR(sb.st_mode) && !S_ISLNK(sb.st_mode))
                                            
        copyfile(writeFile, input);             

}

void archiveend(FILE *writeFile){ 
    unsigned char *buff[BLOCKSIZE];
    memset(&buff, 0, sizeof(buff));
    fwrite(buff, 1, BLOCKSIZE, writeFile);
    fwrite(buff, 1, BLOCKSIZE, writeFile);
}

/*FUNCTIONALITY
    *TRAVERSES THE GIVEN PATH, AND CREATES A TARFILE*/
void traverseAndCreate(FILE* writeFile, char *path, int verbose, int strict){
   DIR *dp;
   struct stat sb;
   struct dirent *contents;
   char newpath[PATH_MAX] = {'\0'};
    
   if(stat(path, &sb) == -1){
        fprintf(stderr, "Path: %s does not exist when traversing directory\n",
                                                                        path);
   }

   if(S_ISDIR(sb.st_mode)){

        if(!(dp = opendir(path))){
            fprintf(stderr, "Failed to open directory\n");
            return;
        }
        /*create the tar for the directory, and then traverse */
        tarfile(writeFile, path, strict);
        if(verbose){
            printf("%s\n", path);
        }
        
        while((contents = readdir(dp)) != NULL){
            if(strcmp(contents->d_name, ".") == 0 ||
                                        strcmp(contents->d_name, "..") == 0)
                continue;

            sprintf(newpath, "%s/%s", path, contents->d_name);
            traverseAndCreate(writeFile, newpath, verbose, strict);
        }

   }else{
        /*if path is reg, just create the tar */
        tarfile(writeFile, path, strict);
        if(verbose){
            printf("%s\n", path);
        }
   }


}


#ifdef DEBUG
int main(void){
    /*header test;
    memset(&test, 0, sizeof(header));
    int t = 1000;
    snprintf(test.mode, 8, "%o", t);
    test.mode[0] = 0;
    printf("%s", test.mode);
    printf("%d", atoi(test.size));*/
    FILE *writeFile = fopen("./test", "w");
    /*createheader(writeFile, "./file"); */
    /*tarfile(writeFile, "./file");*/
    
    traverseAndCreate(writeFile, "./testdir1", 1, 1);
    archiveend(writeFile);
}
#endif
