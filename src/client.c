#include <stdlib.h>
#include <stdio.h>

#include "ctagd.h"

int
main(int argc, char *argv[])
{
	int port = 8200, sock;
	char *hostname = "localhost";
	char buff[256];
	struct sockaddr_in address;
	printf("TODO: Client\n");

	/* Setup Sockets */
	if (!slave_get_sock(port, hostname, &sock, &address)) {
		exit(1);
	}

	read(sock, buff, 256); /* TODO: Figure out how to get rid of this length cap */
	struct smsg *smsg = unpack(buff);
	printf("RECIEVE MSG: %s\n", smsg->payload);
}
