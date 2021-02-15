#include <stdlib.h>
#include <stdio.h>

#include "ctagd.h"

int
main(int argc, char *argv[])
{
	int port = 8200, sock;
	char *hostname = "localhost";
	struct sockaddr_in address;
	printf("[CLIENT] This is a demo!\n");

	/* Setup Sockets */
	if (!slave_get_sock(port, hostname, &sock, &address)) {
		exit(1);
	}

	struct smsg smsg;
	crecv(sock, &smsg);
	printf("RECIEVE MSG\nTag:%c\nMessage:%s\n\n", smsg.tag, smsg.payload);
	crecv(sock, &smsg);
	printf("RECIEVE MSG\nTag:%c\nMessage:%s\n", smsg.tag, smsg.payload);

	// Testing send.
	create_smsg('1', "Hello, World!", &smsg);
	csend(sock, &smsg);
}
