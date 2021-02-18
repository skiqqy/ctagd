#include <stdlib.h>
#include <stdio.h>

#include "ctagd.h"

int
main(int argc, char *argv[])
{
	printf("[CLIENT] This is a demo!\n");

	struct client client;
	init_client_struct(&client); /* Just sets default values, you can still edit the struct as you wish */
	client.enable_q = 1; /* change this to 0 to not use queues */

	/* Setup Sockets */
	if (!init_client(&client)) {
		exit(1);
	}

	struct smsg *smsg;
	if (client.enable_q) {
		smsg = recv_tag('2');
		printf("RECIEVE MSG\nTag:%c\nMessage:%s\n", smsg->tag, smsg->payload);
		smsg = recv_tag('1');
		printf("RECIEVE MSG\nTag:%c\nMessage:%s\n", smsg->tag, smsg->payload);
	} else {
		smsg = malloc(sizeof(struct smsg));
		cfetch(client.socket, smsg);
		printf("RECIEVE MSG\nTag:%c\nMessage:%s\n", smsg->tag, smsg->payload);
		cfetch(client.socket, smsg);
		printf("RECIEVE MSG\nTag:%c\nMessage:%s\n", smsg->tag, smsg->payload);
		free(smsg);
	}

	/* Send a message to server */
	smsg = malloc(sizeof(struct smsg));
	create_smsg('1', "This is a response from the client!", smsg);
	csend(client.socket, smsg);
	free(smsg);
}
