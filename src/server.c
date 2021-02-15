#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ctagd.h"

int
main(int argc, char *argv[])
{
	int client;
	struct server server;
	server.port = 8200;

	printf("[SERVER] This is a demo!\n");

	if (!init_server(&server)) {
		return EXIT_FAILURE;
	}
	client = server_accept();

	struct smsg smsg;
	create_smsg('1', "Hello, World!", &smsg);
	csend(client, &smsg);
	create_smsg('2', "Second Message :D", &smsg);
	csend(client, &smsg);

	// Testing recv
	crecv(client, &smsg);
	printf("RECIEVE MSG\nTag:%c\nMessage:%s\n", smsg.tag, smsg.payload);
}
