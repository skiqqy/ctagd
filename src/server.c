#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ctagd.h"

int
main(int argc, char *argv[])
{
	int client;
	struct server server;
	printf("[SERVER] This is a demo!\n");
	init_server_struct(&server);
	server.max_clients = 1;
	server.enable_q = 1; /* change this to 0 to not use queues */

	if (!init_server(&server)) {
		return EXIT_FAILURE;
	}
	client = server_accept();

	struct smsg *smsg = malloc(sizeof(struct smsg));
	create_smsg('1', "Hello, World!", smsg);
	csend(client, smsg);
	create_smsg('2', "Second Message :D", smsg);
	csend(client, smsg);

	/* Test recv */
	if (server.enable_q) { /* Use the queue */
		free(smsg);
		smsg = recv_tag('1');
	} else {
		cfetch(client, smsg); /* Read the first smsg available from the socket */
	}
	printf("RECIEVE MSG\nTag:%c\nMessage:%s\n", smsg->tag, smsg->payload);
	free(smsg);
}
