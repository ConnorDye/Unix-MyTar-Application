#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include "create.h"

int main(int argc, char* argv[]){
    char *fileName = NULL;
    FILE *writeFile = NULL; /*tar file to write to */
    int opt;
    char *commands = NULL; /*command string for getopt */
    int c = 0, t = 0, x = 0, v = 0, s = 0; /*initialize flags*/
    char *path; /*points to optind argument*/

    
    if(((opt = getopt(argc, argv, "ctxvSf:")) != -1)) {
        fileName = calloc(strlen(optarg) + 1, sizeof(char));
        strcpy(fileName, optarg);
        fprintf(stderr, "Usage: %s [ctxvS]f tarfile [ path [...] ]\n", 
                                                                    argv[0]);
        exit(EXIT_FAILURE);
    } 

    if(optind == argc){
        fprintf(stderr, "Usage: %s [ctxvS]f tarfile [ path [...] ]\n", 
                                                                    argv[0]);
        exit(EXIT_FAILURE);
    }
    
    /*determine the command string */
    commands = argv[optind];
    optind++;
    for(int index = 0; commands[index] != '\0'; index++){
        switch(commands[index]){
        case 'c':
            c = 1;
            break;
        case 't':
            t = 1;
            break;
        case 'x':
            x = 1;
            break;
        case 'v':
            v = 1;
            break;
        case 's':
            s = 1;
            break;
        case 'f':
            /*ignore*/
            break;
        }
    }

    if(c == 0 && x == 0 && t == 0){
        fprintf(stderr, "Usage: %s [ctxvS]f tarfile [ path [...] ]\n", 
                                                                     argv[0]);
    }

    /*open tar file to write to*/
    fileName = argv[optind];
    /*printf("file name is %s\n", fileName); */
    
    if(c){
        writeFile = fopen(fileName, "w");
        optind++;

        /*strict and verbose flags passed */
        while(optind < argc){
            char* path = argv[optind];
            traverseAndCreate(writeFile, path, v, s);
            optind++;
        }
        archiveend(writeFile);
        /*printf("c flag is %d, v flag is %d, s flag is %d", c, v, s);*/
    }
    if(x){
        writeFile = fopen(fileName, "r");
        optind++;

        if(optind < argc){
            while(optind < argc){
                char *path = argv[optind];
                tar_extract(writeFile, path, MODE, v, s);
                optind++;
            }
        }else{ /*if no path given, extract whole archive */
            tar_extract(writeFile, NULL, MODE, v, s);
        }
    }
    if(t){
        writeFile = fopen(fileName, "r");
        optind++;

        if(optind < argc){
            while(optind < argc){
                char *path = argv[optind];
                listarchive(writeFile, path, v, s);
                optind++;
            }
        }else{
            listarchive(writeFile, NULL, v, s);
        }
    }


    return 0;
}
