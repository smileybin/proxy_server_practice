#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <pwd.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <openssl/sha.h>
#include <sys/stat.h>

#define BUFFSIZE 1024 //declare the variable
#define PORTNO 40000

/////////////////////////////////////////////////////////////////////
//File Name     : server.c                        	               //
//Date          : 2018/04/26                                       //
//Os            : Ubuntu 14.04 LTS 64bits                          //
//Author        : Im Hyung Bin                                     //
//Student Id    : 2014722061                                       //
//-----------------------------------------------------------------//
//Title         : System Programming Assignment #2-1 (proxy server)//
//Description   : Clients and server use socket to send message    //
// server use fork to make child process to determine hit or miss  //
/////////////////////////////////////////////////////////////////////

static void handler()
{
	pid_t pid;
	int status;
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0);
}

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

int main()
{
	struct sockaddr_in server_addr, client_addr; //declare the socket variable
	int socket_fd, client_fd;
	int len, len_out;
	char buf[BUFFSIZE];
	pid_t pid;

	char path[128] = {0}; //declare sub process variable
	char logpath[128] = {0};
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

	getHomeDir(path); //get home directory
	strcat(path, "/cache");
	if (opendir(path) == NULL)
	{
		mkdir(path, 0777); //make cache directory
	}

	if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) //if socket not create
	{
		printf("Server:Can't open stream socket."); //print error
		return 0;
	}

	bzero((char *)&server_addr, sizeof(server_addr)); //socket set

	server_addr.sin_family = AF_INET; //server address set
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORTNO);

	if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{													//if socket bind fail
		printf("Server : Can't bind local address.\n"); //print error
		return 0;
	}

	listen(socket_fd, 5); //queue size 5
	signal(SIGCHLD, (void *)handler);

	while (1)
	{
		bzero((char *)&client_addr, sizeof(client_addr)); //socket set
		len = sizeof(client_addr);						  //client_addr size
		client_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &len);

		if (client_fd < 0) //if client_fd is smaller than 0
		{
			printf("Server:accept failed.\n"); //printf error
			close(socket_fd);
			return 0;
		}

		printf("[%s : %d] client was connected\n",inet_ntoa(*(struct in_addr*)&client_addr.sin_addr), client_addr.sin_port);//print txt

		pid = fork(); //use fork function to make child process

		if (pid == -1)
		{
			close(client_fd);
			close(socket_fd);
			continue;
		}
		else if (pid == 0)
		{
			char hash_url[128] = {0}; //sub process function
			char d_name[4] = {0};
			char f_name[128] = {0};
			int hitnum = 0;
			int missnum = 0;
			int fnum = 0;
			int hit=0;
			int i=0;
			DIR *dp;
			struct dirent *dirp;

			int diff_time = 0;
			time_t start, end, now;
			time(&start);
			struct tm *ltp;
			bzero(buf,BUFFSIZE); //initialize the buf
			while ((len_out = read(client_fd, buf, BUFFSIZE)) > 0)
				{
					buf[strlen(buf)-1]='\0'; //set NULL in buf

					if (strncmp(buf, "bye", 3) == 0)break; //if bye break;
					
					else
					{
						sha1_hash(buf, hash_url);

						for (i = 0; i < 3; i++)
						{
							d_name[i] = hash_url[i]; //get directory name
						}
						fnum = 0;
						for (i = 3; i < strlen(hash_url); i++)
						{
							f_name[fnum] = hash_url[i]; //get file name
							fnum++;
						}

						getHomeDir(path);
						strcat(path, "/cache");
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
							time(&now);
							ltp = localtime(&now);//print text and send socket to client
							log = fopen(logpath, "a");
							fprintf(log, "[MISS] ServerPID : %d | %s - [%d/%d/%d,%02d:%02d:%02d] \n", getpid(), buf, 1900 + ltp->tm_year, ltp->tm_mon + 1, ltp->tm_mday, ltp->tm_hour, ltp->tm_min, ltp->tm_sec);
							fclose(log);
							write(client_fd,"MISS\n",strlen("MISS\n"));
							hit = 0;
							bzero(buf,BUFFSIZE);
							
						}
						else //hit
						{
							hitnum++;
							time(&now);
							ltp = localtime(&now);//print text and send socket to client
							log = fopen(logpath, "a");
							fprintf(log, "[HIT] ServerPID : %d | %s/%s - [%d/%d/%d,%02d:%02d:%02d] \n", getpid(), d_name, f_name, 1900 + ltp->tm_year, ltp->tm_mon + 1, ltp->tm_mday, ltp->tm_hour, ltp->tm_min, ltp->tm_sec);
							fprintf(log, "[HIT]%s\n", buf);
							fclose(log);
							write(client_fd,"HIT\n",strlen("HIT\n"));
							hit = 0; 
							bzero(buf,BUFFSIZE);
						}
					}
				}
			time(&end);
			diff_time = difftime(end, start);
			log = fopen(logpath, "a");
			fprintf(log, "[Terminated] ServerPid : %d | run time: %d sec. #request hit: %d #miss: %d \n", getpid(), diff_time, hitnum, missnum);
			printf("[%s : %d] client was disconnected\n",inet_ntoa(*(struct in_addr*)&client_addr.sin_addr), client_addr.sin_port);
			close(client_fd);//close client
			fclose(log); //close file log
			return 0;
		}
		close(client_fd); //close client
	}
	close(socket_fd);//close socket
	return 0;
}