/////////////////////////////////////////////////////////////////////
//File Name     : proxy_cache.c                                    //
//Date          : 2018/03/22                                       //
//Os            : Ubuntu 14.04 LTS 64bits                          //
//Author        : Im Hyung Bin                                     //
//Student Id    : 2014722061                                       //
//-----------------------------------------------------------------//
//Title         : System Programming Assignment #1-1 (proxy server)//
//Description   :                                                  //
/////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

void main(int argc, char *argv[])
{
	if(argc<2){
		printf("error\n");
		return;
	}
	umask(0);
	mkdir(argv[1], S_IRWXU|S_IRWXG|S_IRWXO);
}


