#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ctagd.h"

int
main(int argc, char *argv[])
{
	int server_fd, client;
	int port = 8200;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	printf("[SERVER] This is a demo!\n");

	if (!master_open_socket(&server_fd, &address, OPT, 1, port)) {
		return EXIT_FAILURE;
	}
	client = accept(server_fd, (struct sockaddr *) &address, (socklen_t *) &addrlen);
	printf("Server socket opened, port = %d\nserver_fd = %d\nAccepting Cons.\n", port, server_fd);

	char *msg = "Hello, World!";
	char *bmsg = pack(create_smsg('t', msg));
	printf("Sending message: %s\n", msg);
	printf("strln(bmsg) = %d\n", strlen(bmsg));
	send(client, bmsg, 5 + strlen(msg), 0);
}
