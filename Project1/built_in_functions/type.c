#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "../parser.h"
#include <dirent.h>
#include <unistd.h>

static char * builtIns[] = {"?","cd","exit","kill","pwd","ulimit","nice","echo","type","export"};
static char * directories[100];
int containsStrr(char * str){
    for(int i=0; i<10; i++){
        if(strcmp(builtIns[i],str) == 0) return 1;
    }
    return -1;
}

void getDirectories(char * allDir){
    int counter = 0;
    int last = 0;
    char * dir[150];
    int size = strlen(allDir);
    memcpy(dir,allDir,size);
    for(int i=0; i<size;i++){
        if(allDir[i] == ':'){
            char * dest = malloc(200);
            memcpy(dest,allDir+last,i-last);
            //printf("%s\n",dest );
            directories[counter] = dest;
            last = i+1;
            counter++;
        }
    }
    char * finalDir = malloc(200);
    memcpy(finalDir,allDir+last,size-last);
    directories[counter++]=finalDir;
    directories[counter]="--f";
}


int type(command_explained  * cex){
    if(cex == NULL) return -1;
    char * s;
    while((s = next_parameter_value(cex)) != NULL){
        if(containsStrr(s)){
            write(STDOUT_FILENO, s, strlen(s));
            char *tmp = " is a shell builtin\n";
            write(STDOUT_FILENO, tmp, strlen(tmp));
        }
        getDirectories(getenv("PATH"));
        DIR *dir;
        struct dirent *ent;
        int i=0;
        while(1) {
            if(strcmp(directories[i],"--f") == 0)
                break;
            if ((dir = opendir(directories[i])) != NULL) {
                while ((ent = readdir(dir)) != NULL) {
                    if(strcmp(ent->d_name,s) == 0){
                        write(STDOUT_FILENO, s, strlen(s));
                        write(STDOUT_FILENO, " is ", 4);
                        write(STDOUT_FILENO, directories[i], strlen(directories[i]));
                        write(STDOUT_FILENO, "/", 1);
                        write(STDOUT_FILENO, s, strlen(s));
                        write(STDOUT_FILENO, "\n", 1);
                        //strcat(res,"-- this command is program");
                    }
                }
                closedir(dir);
            }
            i++;
        }
        return 0;
    }
    return -1;
}