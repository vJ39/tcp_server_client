#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main(int argc, char **argv){
    char **makearg;
    int i;
    for(i = 0;i <= argc;i++){
        if(i == argc) makearg[i] = NULL;
        else makearg[i] = argv[i+1];
    }
    if(execve(makearg[0], makearg, NULL) == -1)perror("");
    return 0;
}
