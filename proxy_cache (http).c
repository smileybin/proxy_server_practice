#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <dirent.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <pwd.h>

#define BUFFSIZE   1024
#define PORTNO     39999



/////////////////////////////////////////////////////////////////////
//File Name     : proxy_cache.c                       	           //
//Date          : 2018/05/01                                       //
//Os            : Ubuntu 14.04 LTS 64bits                          //
//Author        : Im Hyung Bin                                     //
//Student Id    : 2014722061                                       //
//-----------------------------------------------------------------//
//Title         : System Programming Assignment #2-2 (proxy server)//
//Description   : Forward HTTP request to Web server and take url  //
// to determine hit or miss and print the HTTP response            //
/////////////////////////////////////////////////////////////////////


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
  struct sockaddr_in server_addr, client_addr;
  int socket_fd,client_fd;
  int len, len_out;
  char path[128] = {0}; //declare sub process variable
  pid_t pid;
 
  getHomeDir(path); //get home directory
  strcat(path, "/cache");
  if (opendir(path) == NULL)
  {
   mkdir(path, 0777); //make cache directory
  }
   
   if((socket_fd = socket(PF_INET, SOCK_STREAM, 0))<0){ //if socket making failed , return 
        printf("Server : Can't open stream socket.");
        return 0;
   }
   
   bzero((char *)&server_addr, sizeof(server_addr)); // initialize server and set server address
   server_addr.sin_family = AF_INET; 
   server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
   server_addr.sin_port = htons(PORTNO); 

   int opt=1;
   setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
   
   if(bind(socket_fd, (struct sockaddr *)&server_addr,sizeof(server_addr)) < 0){//if bind failed return 0;
   printf("Server : Can't bind local address.\n");
   close(socket_fd); // close socket_fd
   return 0;
   }

   listen(socket_fd, 5);
   while(1){
        struct in_addr inet_client_address; //declare the response and server variable
        char buf[BUFFSIZE];
        char response_header[BUFFSIZE] = {0, };
        char response_message[BUFFSIZE] = {0, };
   
        char tmp[BUFFSIZE] = {0, };
        char method[20] = {0, };
        char url[BUFFSIZE] = {0, };

        char *tok = NULL;

        bzero(response_header,BUFFSIZE);
        bzero(response_message,BUFFSIZE);
        bzero(tmp,BUFFSIZE);
        bzero(url,BUFFSIZE);
        bzero(buf,BUFFSIZE);
       

        len = sizeof(client_addr);
        client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &len);
        if(client_fd<0)
        { //if suver not accept print error and return 0;
            printf("Server : accept failled\n");
            return 0;
        }
        inet_client_address.s_addr = client_addr.sin_addr.s_addr;

       
        printf("[%s : %d] client was connected\n", inet_ntoa(inet_client_address), client_addr.sin_port);
        read(client_fd, buf, BUFFSIZE);
        strcpy(tmp, buf);
        puts("=======================================");
        printf("Request from [%s : %d]\n", inet_ntoa(inet_client_address), client_addr.sin_port);
        puts(buf);
        puts("=======================================\n"); //print request message
        tok = strtok(tmp, " ");
        strcpy(method, tok);

        if (strcmp(method, "GET") == 0)
        {
        //strtok to take url
        tok = strtok(NULL, " ");
        strcpy(url, tok);
        }
        else continue;
        
        if(url[strlen(url)-1]='/'){
        url[strlen(url)-1]='\0';
        }
        
        pid = fork(); //use fork function to make child process

        if (pid == -1) //if fork failed continue
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
          int hit=0;
          int fnum=0;
          int i=0;
          DIR *dp;
          struct dirent *dirp;

          sha1_hash(url, hash_url);

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
              
              sprintf(response_message,"Miss\n");
              sprintf(response_header,
                  "HTTP/1.0 200 OK \r\n"
                  "Server:2018 simple web server \r\n"
                  "Content-length:%lu\r\n"
                  "Content-type:text/html\r\n\r\n",
                  strlen(response_message));
              write(client_fd, response_header, strlen(response_header));
              write(client_fd, response_message, strlen(response_message));
							hit = 0;
						}
						else //hit
						{
							sprintf(response_message,"Hit\n");
              sprintf(response_header,
                  "HTTP/1.0 200 OK \r\n"
                  "Server:2018 simple web server \r\n"
                  "Content-length:%lu\r\n"
                  "Content-type:text/html\r\n\r\n",
                  strlen(response_message));
              write(client_fd, response_header, strlen(response_header));
              write(client_fd, response_message, strlen(response_message));
              hit = 0; 
						}
            return 0;
        
        }
          printf("[%s : %d] client was disconnected\n", inet_ntoa(inet_client_address), client_addr.sin_port);
          close(client_fd);
   }
        close(socket_fd);
        return 0;
}
   





