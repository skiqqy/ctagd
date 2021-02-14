#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ctagd.h"

static int max_clients;

static int
bytes_to_int(char *bytes)
{
	return bytes[0] + (bytes[1] << 8) + (bytes[2] << 16) + (bytes[3] << 24);
}

static char *
int_to_bytes(int i)
{
	char *bytes = malloc(sizeof(char)*4); // We add a null terminator
	int j;
	for (j = 3; j >= 0; j--) {
		bytes[j] = (i >> 4 * i) & 0xFF;
	}
	printf("TESTING: %d=?=%d\n", i, bytes_to_int(bytes));
	return bytes;
}


/* The master opens its socket to allow clients to communicate
  
 * int *sfd: Socket File Descriptor.
 * struct sockaddr_in *address: Pointer to address struct.
 * int opt: The option value.
 * int max: Max client connections.
 * int port: The port we are opening.
  
*/
int
master_open_socket(int *sfd, struct sockaddr_in *address, int opt, int max, int port)
{
	if (!(*sfd = socket(AF_INET, SOCK_STREAM, 0))) {
		perror("socket failed!");
	}

	if (setsockopt(*sfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
		perror("setsockopt"); 
		return 0;
	}

	address->sin_family = AF_INET;
	address->sin_addr.s_addr = INADDR_ANY;
	address->sin_port = htons(port);

	if (bind(*sfd, (struct sockaddr *) address, sizeof(*address)) < 0) {
		perror("bind fail!");
		return 0;
	}

	max_clients = max;
	if (listen(*sfd, max_clients) < 0) {
		perror("listen fail!");
		return 0;
	}

	// Success
	return 1;
}

int
slave_get_sock(int port, char *host, int *sock, struct sockaddr_in *address)
{
	struct hostent *server;
	struct in_addr **list;

	/* First resolve the hostname */
	if ((server = gethostbyname(host)) == NULL) {
		perror("ERROR: Resolving hostname failed.\n");
		return 0;
	}

	list = (struct in_addr **) server->h_addr_list;
	if (!list[0]) {
		perror("ERROR: Hostname list empty.\n");
		return 0;
	}

	if ((*sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("ERROR: Socket creation error.\n");
		return 0;
	}

	address->sin_family = AF_INET;
	address->sin_port = htons(port);

	if (inet_pton(AF_INET, inet_ntoa(*list[0]), &address->sin_addr) <= 0) {
		perror("ERROR: Invalid address/Address not supported.\n");
		return 0;
	}

	if (connect(*sock, (struct sockaddr *) address, sizeof(*address)) < 0) {
		perror("ERROR: Connection Failed.\n");
		return 0;
	}

	/* Success */
	return 1;
}

struct smsg *
create_smsg(char tag, char *msg)
{
	if (msg == NULL) return NULL;
	struct smsg *smsg = malloc(sizeof(struct smsg));
	smsg->tag = tag;
	smsg->len = strlen(msg);
	smsg->payload = msg;
	return smsg;
}

/* Convert an smsg to a byte array (bmsg).
 * struct smsg: Convert the smsg (Struct message) into a bmsg (a byte array)
 */
char *
pack(struct smsg *msg)
{
	if (msg == NULL) return NULL;
	char *bmsg = malloc(sizeof(char)*msg->len + 5); /* first byte = tag, next 4 bytes = len */
	char *len = int_to_bytes(msg->len);
	bmsg [0] = msg->tag;
	sprintf(&bmsg[1], "%s%s", len, msg->payload);
	printf("[PACK] %s\n", bmsg);
	free(len);
	return bmsg;
}

/* Convert a byte array message to a struct message (smsg)
 * char *bmsg: The byte array message we are converting.
 */
struct smsg *
unpack(char *bmsg)
{
	if (bmsg == NULL) return NULL;
	struct smsg *smsg = malloc(sizeof(struct smsg));
	sscanf(bmsg, "%c", &smsg->tag);
	smsg->len = bytes_to_int(&bmsg[1]);
	smsg->payload = malloc(sizeof(char)*smsg->len);
	sprintf(smsg->payload, "%s", &bmsg[5]);
	return smsg;
}
