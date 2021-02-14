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

#define VERSION 0.1
#define OPT 1

/* Tagged message struct */
struct smsg {
	char tag;
	int len;
	char *payload;
};

int master_open_socket(int *sfd, struct sockaddr_in *address, int opt, int max, int port);
int slave_get_sock(int port, char *host, int *sock, struct sockaddr_in *address);
char *pack(struct smsg *message);
struct smsg *unpack(char *bmsg);
struct smsg *create_smsg(char tag, char *msg);
