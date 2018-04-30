/////////////////////////////////////////////////////////////////////
//File Name     : proxy_cache.c                                    //
//Date          : 2018/04/13                                       //
//Os            : Ubuntu 14.04 LTS 64bits                          //
//Author        : Im Hyung Bin                                     //
//Student Id    : 2014722061                                       //
//-----------------------------------------------------------------//
//Title         : System Programming Assignment #1-3 (proxy server)//
//Description   : To use fork function to make child process       //
//              (insert url function)and take entire execution time//
/////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <openssl/sha.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

char *sha1_hash(char *input_url, char *hashed_url)
{ //hash function
    unsigned char hashed_160bits[20];
    char hashed_hex[41];
    int i;

    SHA1(input_url, strlen(input_url), hashed_160bits);

    for (i = 0; i < sizeof(hashed_160bits); i++)
        sprintf(hashed_hex + i * 2, "%02x", hashed_160bits[i]);

    strcpy(hashed_url, hashed_hex);

    return hashed_url;
}

char *getHomeDir(char *home)
{ //gethome directory function
    struct passwd *usr_info = getpwuid(getuid());
    strcpy(home, usr_info->pw_dir);

    return home;
}

int log_write(char *filename, char *dirname, int check, char *inserted_url, char *logpath2)
{
    FILE *log;
    log = fopen(logpath2, "a");
    time_t Time;
    time(&Time);
    struct tm *f_time;
    f_time = localtime(&Time); //take local time

    if (check == 0) //miss
    {
        fprintf(log, "[Miss]%s-[%d/%02d/%02d, %02d:%02d:%02d]\n",
        inserted_url, 1900 + f_time->tm_year, f_time->tm_mon + 1, f_time->tm_mday, f_time->tm_hour, f_time->tm_min, f_time->tm_sec);
    }

    else if (check == 1) //hit
    {
        fprintf(log, "[Hit]%s/%s-[%d/%02d/%02d, %02d:%02d:%02d]\n[Hit]%s\n",
        dirname, filename, f_time->tm_year + 1900, f_time->tm_mon + 1, f_time->tm_mday, f_time->tm_hour, f_time->tm_min, f_time->tm_sec, inserted_url);
    }

    fclose(log);
    return 0;
}

int sub_process(FILE *log2, char *logpath3)
{
    char path[128] = {0};
    char url[128] = {0};
    char hash_url[128] = {0};
    char d_name[4] = {0};
    char f_name[128] = {0};
    int fnum = 0;
    int diff_time = 0;
    int hit = 0;
    int i = 0;
    int hitnum = 0;
    int missnum = 0;

    time_t start, end;
    time(&start);

    DIR *dp;
    struct dirent *dirp;

    while (1)
    {
        fnum = 0;
        printf("[%d]", getpid());
        fputs("Input URL> ", stdout); //insert URL
        fgets(url, 128, stdin);

        url[strlen(url) - 1] = '\0'; //set NULL in last element

        getHomeDir(path); //get home directory
        strcat(path, "/cache");
        if (opendir(path) == NULL)
        {
            mkdir(path, 0777); //make cache directory
        }

        if (strcmp(url, "bye") == 0)
            break; //if bye command insert exit function

        sha1_hash(url, hash_url);

        for (i = 0; i < 3; i++)
        {
            d_name[i] = hash_url[i]; //get directory name
        }

        for (i = 3; i < strlen(hash_url); i++)
        {
            f_name[fnum] = hash_url[i]; //get file name
            fnum++;
        }

        strcat(path, "/");
        strcat(path, d_name);
        dp = opendir(path); //take /cache/d_name directory

        if (dp != NULL) //if dp != NULL
        {
            for (dirp = readdir(dp); dirp; dirp = readdir(dp))
            {
                if (strcmp(dirp->d_name, f_name) == 0) //if there is same directory, hit
                {
                    hit = 1;
                    break;
                }
            }
        }

        if (hit == 0) //miss
        {
            umask(0);
            mkdir(path, 0777); //make directory

            strcat(path, "/");
            strcat(path, f_name);
            creat(path, 0777); //make file
            missnum++;
            log_write(NULL, NULL, hit, url, logpath3); //log write
            hit = 0;
        }
        else
        {
            hitnum++;
            log_write(f_name, d_name, hit, url, logpath3); //log write
            hit = 0;                                       //hit=0 and do nothing
        }
    }

    if (strcmp(url, "bye") != 0)
    {
        closedir(dp); //close dp
    }

    time(&end);                       //take end time
    diff_time = difftime(end, start); //take execution time

    log2 = fopen(logpath3, "a"); //log open and write
    fprintf(log2, "[Terminated] run time: %d sec. #request hit : %d, miss : %d\n", diff_time, hitnum, missnum);
}

int main()
{
    char cmd[128] = {0};
    char logpath[128] = {0};

    time_t p_start;
    time_t p_end;
    int p_diff_time = 0;
    int p_num = 0;

    time(&p_start);

    pid_t pid; //make process valuable
    pid_t pid_child;
    int status;
    DIR *dp;

    FILE *log;
    getHomeDir(logpath);
    strcat(logpath, "/logfile");
    umask(0);

    if (opendir(logpath) == NULL)
    {
        mkdir(logpath, 0777); //make logpath
    }

    strcat(logpath, "/logfile.txt"); //open logfile
    log = fopen(logpath, "a");

    while (1)
    {
        pid = getpid(); //get pid numver
        printf("[%d]", pid); 
        fputs("Input CMD> ", stdout);
        fgets(cmd, 128, stdin);

        cmd[strlen(cmd) - 1] = '\0';

        if (strcmp(cmd, "quit") == 0) //if quit break
        {
            break;
        }

        else if (strcmp(cmd, "connect") == 0)
        {
            p_num++;      //process num plus
            pid = fork(); //take child process

            if (pid == 0)
            {
                sub_process(log, logpath); //child(sub) process
                return 0;
            }
            else if (pid < 0)
            {
                printf("fork error \n");
                p_num--;
            }
            else
            {
                pid_child = waitpid(pid, &status, 0); //wait function

                if (status == 0)
                {
                    printf("Normal Exit \n");
                }
            }
        }
    }

    time(&p_end);
    p_diff_time = difftime(p_end, p_start); //take entire program execution time

    fprintf(log, "**SERVER** [Terminated] run time: %d sec. #sub process: %d \n", p_diff_time, p_num);

    return 0;
}