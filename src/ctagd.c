#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "ctagd.h"

#define QUEUE_SIZE 256

struct queue {
	struct smsg *smsg;
	struct queue *next;
};

static struct server *server = NULL;
static struct client *client = NULL;
static struct queue *queue[QUEUE_SIZE];
static int *connected;
static pthread_t *recv_pt; /* Each connected client gets thier own thread */

/* Enqueue an smsg into a queue
  
 * struct queue **q: The queue we are adding too.
 * struct smsg *smsg: The smsg we are adding.
  
 */
void
enqueue(struct queue **q, struct smsg *smsg)
{
	struct queue *insert = malloc(sizeof(struct queue));
	struct queue *t = *q;
	insert->smsg = smsg;
	insert->next = NULL;

	/* TODO: only lock the index that is being written to */
	if (*q == NULL) {
		*q = insert;
	} else {
		while (t->next != NULL) {
			t = t->next;
		}
		t->next = insert; /* insert at the end of the q */
	}
}

/* Dequeue an smsg from a q.
  
 * struct queue **q: The queue we are removing from.
 * return struct smsg *: A pointer to the smsg, null if the queue is empty.
  
 */
struct smsg *
dequeue(struct queue **q)
{
	struct smsg *ret;
	if (*q == NULL) return NULL; /* Empty q */

	ret = (*q)->smsg;
	*q = (*q)->next;

	return ret;
}

/* This thread recv's messages from a socket and puts them in thier appropraite
 * queues.
  
 * void *s: index into connected array for socket fd.
  
 */
void *
recv_thread(void *s)
{
	struct smsg *smsg;
	int id = *((int *) s);
	
	for(;;) {
		/* TODO: Add locks */
		while (connected[id] == -1) {}; /* While this thread has no connected client do nothing. */
		smsg = malloc(sizeof(struct smsg));
		if (!crecv(connected[id], smsg)) {
			/* TODO: Lock this */
			connected[id] = -1; /* Client disconnected */
		}

		/* TODO: Lock this */
		enqueue(&queue[(int) smsg->tag], smsg); /* We enqueue the msg into its tagged q */
	}

	return 0;
}

/* Init and open the server to allow clients to connect
  
 * struct server *s: The server Struct.
  
 */
int
init_server(struct server *s)
{
	int i;
	server = s;
	if (!server_open_socket(&s->server_fd, &s->address, OPT, s->port)) {
		return 0;
	}
	for (i = 0; i < QUEUE_SIZE; i++) {
		queue[i] = NULL;
	}

	connected = malloc(sizeof(int)*s->max_clients);
	recv_pt = malloc(sizeof(pthread_t)*s->max_clients);
	for (i = 0; i < s->max_clients; i++) {
		connected[i] = -1; /* Means the slot is free */
	}

	return 1;
}

int
init_client(struct client *c)
{
	client = c;
	if (!client_get_sock(client->port, client->hostname, &client->socket, &client->address)) {
		return 0;
	}
	return 1;
}

int
server_accept()
{
	if (server == NULL) return -1;
	int addrlen = sizeof(server->address);
    int soc = accept(server->server_fd, (struct sockaddr *) &server->address, (socklen_t *) &addrlen);
	int i;
	
	/* TODO: Add locks */
	for (i = 0; i < server->max_clients; i++) {
		if (connected[i] == -1) {
			connected[i] = soc;
			break;
		}
	}

	return soc;
}

/* The master opens its socket to allow clients to communicate
  
 * int *sfd: Socket File Descriptor.
 * struct sockaddr_in *address: Pointer to address struct.
 * int opt: The option value.
 * int max: Max client connections.
 * int port: The port we are opening.
 * return: 1 for success, 0 for fail.

*/
int
server_open_socket(int *sfd, struct sockaddr_in *address, int opt, int port)
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

	if (listen(*sfd, server->max_clients) < 0) {
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
client_get_sock(int port, char *host, int *sock, struct sockaddr_in *address)
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
	buff = realloc(buff, sizeof(char)*len + 5 + 1); /* len = data length, 5 = tag + len, 1 = null term */
	read(sock, &buff[5], len);
	buff[5 + len] = 0; /* We append a null term just incase we work with a string */
	return unpack(buff, smsg);
}
