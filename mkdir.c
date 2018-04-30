#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    struct dirent *pFile;
    DIR *pDir;

    if(argc < 2) return -1;

    pDir=opendir(argv[1]);

    if(pDir == NULL){
        printf("Dir read error\n");
        return -1;
    }
    for(pFile = readdir(pDir); pFile; pFile = readdir(pDir))
        printf("%s\n", pFile->d_name);
    closedir(pDir);
    return 1;
}