#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ctagd.h"

static int max_clients;

/* The master opens its socket to allow clients to communicate
  
 * int *sfd: Socket File Descriptor.
 * struct sockaddr_in *address: Pointer to address struct.
 * int opt: The option value.
 * int max: Max client connections.
 * int port: The port we are opening.
 * return: 1 for success, 0 for fail.

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

/* Connect the client to the server and get a socket fd.
  
 * int port: The port we are connecting to.
 * char *host: Hostname of the server.
 * int *sock: pointer to where we will store the socket fd.
 * struct sockaddr_in *address: Address socket.
 * return: 1 for success, 0 for fail.
  
 */
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

/* Take a tag and a message and pack it into an smsg
  
 * char tag: The tag for the message.
 * char *msg: the msg.
 * return: 1 for success, 0 for fail.
  
 */
int
create_smsg(char tag, char *msg, struct smsg *smsg)
{
	if (msg == NULL) return 0;
	smsg->tag = tag;
	smsg->len = strlen(msg);
	smsg->payload = msg;
	return 1;
}

/* Convert an smsg to a byte array (bmsg).
  
 * struct smsg: Convert the smsg (Struct message) into a bmsg (a byte array)
 * return: 1 for success, 0 for fail.
  
 */
char *
pack(struct smsg *msg)
{
	if (msg == NULL) return NULL;
	char *bmsg = malloc(sizeof(char)*msg->len + 5); /* first byte = tag, next 4 bytes = len */
	char *bint = (char*) &msg->len;

	/* Pack the byte array */
	bmsg[0] = msg->tag;
	int i;
	for (i = 1; i < 5; i++) {
		bmsg[i] = bint[i-1];
	}
	sprintf(&bmsg[5], "%s", msg->payload);

	return bmsg;
}

/* Convert a byte array message to a struct message (smsg)
  
 * char *bmsg: The byte array message we are converting.
 * return: 1 for success, 0 for fail.
  
 */
int
unpack(char *bmsg, struct smsg *smsg)
{
	if (bmsg == NULL) return 0;

	smsg->tag = bmsg[0];
	smsg->len = *(int *)&bmsg[1];
	smsg->payload = malloc(sizeof(char)*smsg->len);
	sprintf(smsg->payload, "%s", &bmsg[5]);

	return 1;
}

/* Send a smsg over a socket.
  
 * int sock: The socket we are sending over.
 * struct smsg *smsg: Pointer to the smsg we are sending.
 * return: 1 for success, 0 for fail.
  
 */
int
csend(int sock, struct smsg *smsg)
{
	char *bmsg = pack(smsg);
	if (bmsg == NULL) return 0;

	send(sock, bmsg, 5 + smsg->len, 0);
	free(bmsg); /* This might cause a seggy */

	return 1;
}

/* Read a smsg from a socket.
  
 * int sock: The socket we are reading from.
 * struct smsg *smsg: Pointer to the smsg we are stroing the message in.
 * return: 1 for success, 0 for fail.
  
 */
int
crecv(int sock, struct smsg *smsg)
{
	char *buff = malloc(sizeof(char)*5);
	read(sock, buff, 5);
	int len = *(int *) &buff[1];
	buff = realloc(buff, sizeof(char)*len + 5);
	read(sock, &buff[5], len);
	return unpack(buff, smsg);
}
