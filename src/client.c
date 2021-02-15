#include <stdlib.h>
#include <stdio.h>

#include "ctagd.h"

int
main(int argc, char *argv[])
{
	printf("[CLIENT] This is a demo!\n");

	struct client client;
	client.port = 8200;
	client.hostname = "localhost";

	/* Setup Sockets */
	if (!init_client(&client)) {
		exit(1);
	}

	struct smsg smsg;
	crecv(client.socket, &smsg);
	printf("RECIEVE MSG\nTag:%c\nMessage:%s\n\n", smsg.tag, smsg.payload);
	crecv(client.socket, &smsg);
	printf("RECIEVE MSG\nTag:%c\nMessage:%s\n", smsg.tag, smsg.payload);

	// Testing send.
	create_smsg('1', "Hello, World!", &smsg);
	printf("SENDING:%s\n", smsg.payload);
	csend(client.socket, &smsg);
}
