#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include "header.h"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Invalid parameter\n");
		return -1;
	}

	//char *init_url = "http://www.ibdhost.com/help/html/";
	//char *init_url = "http://arkieprinceblog.com/";

	char *init_url = argv[1];

	StartCatch(init_url);

	return 0;
}
