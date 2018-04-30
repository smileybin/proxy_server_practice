#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFSIZE	1024//define buffsize & portnum
#define PORTNO		40000

/////////////////////////////////////////////////////////////////////
//File Name     : client.c                        	               //
//Date          : 2018/04/26                                       //
//Os            : Ubuntu 14.04 LTS 64bits                          //
//Author        : Im Hyung Bin                                     //
//Student Id    : 2014722061                                       //
//-----------------------------------------------------------------//
//Title         : System Programming Assignment #2-1 (proxy server)//
//Description   : Client inserts url and use socket to send url    //
// 																   //
/////////////////////////////////////////////////////////////////////

int main()
{
	int socket_fd, len;
	struct sockaddr_in server_addr;
	char haddr[]="127.0.0.1";//haddr is 127.0.0.1 
	char buf[BUFFSIZE];
	
	if((socket_fd=socket(PF_INET, SOCK_STREAM,0))<0) //if socket creation fail
	{
		printf("can't create socket.\n");
		return -1;//print error and exit
	}

	bzero(buf,sizeof(buf));
	bzero((char*)&server_addr,sizeof(server_addr));//socket set

	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=inet_addr(haddr);
	server_addr.sin_port=htons(PORTNO);
	//server_addr set

	if(connect(socket_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0)//if connect is failed
	{
		printf("can't connect.\n");//print error and exit
		return -1;
	}

	write(STDOUT_FILENO, "Input url > ", strlen("Input url > "));
	while((len=read(STDIN_FILENO, buf,sizeof(buf)))>0) //while len reading buf
	{
		if(strncmp(buf,"bye",3)==0)
		{
		write(socket_fd,buf,BUFFSIZE); //send bye and break;
		break;
		}
		if(write(socket_fd,buf,strlen(buf))>0) //if write funct 
		{
		if((len=read(socket_fd,buf,sizeof(buf)))>0) //read is success 
			{
			write(STDOUT_FILENO,buf,len);//write and buf set
			bzero(buf,sizeof(buf));
			}
		}
	write(STDOUT_FILENO, "Input url > ", strlen("Input url > "));
	}
	close(socket_fd);//socket_fd close
	return 0;
}

