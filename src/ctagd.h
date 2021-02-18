#include <stdlib.h>
#include <stdio.h>

#include <unistd.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <netinet/in.h> 
#include <netdb.h>
#include <string.h> 
#include <errno.h>
#include <signal.h>

#define VERSION 1.0
#define OPT 1

/* Tagged message struct */
struct smsg {
	char tag;
	int len;
	char *payload;
};

struct server {
	int server_fd;
	int port;
	int max_clients;
	int enable_q;
	struct sockaddr_in address;
};

struct client {
	int socket;
	int port;
	char *hostname;
	int enable_q;
	struct sockaddr_in address;
};

/* Init */
void init_server_struct(struct server *server);
void init_client_struct(struct client *client);
int init_server(struct server *server);
int init_client(struct client *client);
int server_open_socket(int *sfd, struct sockaddr_in *address, int opt, int port);
int client_get_sock(int port, char *host, int *sock, struct sockaddr_in *address);

/* Connection Establishment */
int server_accept(); /* Server accepts client con */

/* Message Building */
char *pack(struct smsg *message);
int unpack(char *bmsg, struct smsg *smsg);
int create_smsg(char tag, char *msg, struct smsg *smsg);

/* Message Passing */
int csend(int sock, struct smsg *smsg);
int cfetch(int sock, struct smsg *smsg);
struct smsg * recv_tag(char tag); /* This is used to recv messages */
