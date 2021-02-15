#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ctagd.h"

int
main(int argc, char *argv[])
{
	int server_fd, client;
	int port = 8200;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	printf("[SERVER] This is a demo!\n");

	if (!master_open_socket(&server_fd, &address, OPT, 1, port)) {
		return EXIT_FAILURE;
	}
	client = accept(server_fd, (struct sockaddr *) &address, (socklen_t *) &addrlen);
	printf("Server socket opened, port = %d\nserver_fd = %d\nAccepting Cons.\n", port, server_fd);

	struct smsg smsg;
	create_smsg('1', "Hello, World!", &smsg);
	csend(client, &smsg);
	create_smsg('2', "Second Message :D", &smsg);
	csend(client, &smsg);

	// Testing recv
	crecv(client, &smsg);
	printf("RECIEVE MSG\nTag:%c\nMessage:%s\n", smsg.tag, smsg.payload);
}
