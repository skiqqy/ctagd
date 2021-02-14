#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ctagd.h"

static int max_clients;

/* I didnt feel like working with bytes, leave me alone owo */
static int base_10(int n)
{
	int c = 0;
	while (n > 0) {
		n /= 10;
		c++;
	}

	return c;
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
	bmsg [0] = msg->tag;
	sprintf(&bmsg[1], "%d%s", msg->len, msg->payload);
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
	sscanf(bmsg, "%c%d", &smsg->tag, &smsg->len);
	smsg->payload = malloc(sizeof(char)*smsg->len);
	sprintf(smsg->payload, "%s", &bmsg[1+base_10(smsg->len)]);
	return smsg;
}
