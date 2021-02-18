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

	struct smsg *smsg = NULL;
	while (smsg == NULL) {
		smsg = recv_tag('2');
	}
	printf("RECIEVE MSG\nTag:%c\nMessage:%s\n", smsg->tag, smsg->payload);
	smsg = NULL;
	while (smsg == NULL) {
		smsg = recv_tag('1');
	}
	printf("RECIEVE MSG\nTag:%c\nMessage:%s\n", smsg->tag, smsg->payload);

	/* Send a message to server */
	smsg = malloc(sizeof(struct smsg));
	create_smsg('1', "This is a response from the client!", smsg);
	csend(client.socket, smsg);
	free(smsg);
}
