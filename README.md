# ctagd
ctagd is a tag based message passing interface over sockets (MPIS for short).

## Design
ctagd works by having a master program (also called a server) and communicates
with several slave programs (also called clients).

Currently there are two modes:
The first is a queue based mode where each tagged message gets put in its own
queue, in this mode the server has a thread for each client and recvs messages
and puts the messages into a queue (based on thier tags). Similarly the client
has its own queue and thread that does the same.

The second mode is just a simple fifo read from the socket, that is you specify
which socket you wish to read from and then read the first message from the
socket.

TODO: write better documentation on the features of ctagd and how to use them.

## Building
To just build the object file, run:
````
$ make build
````
If you want to build the library, then run:
````
$ make lib
````
Note when including this library in your own project it is important to link
threads (like so `-lpthread`) when compiling.

## Installing
TODO

## Testing
````
$ make test
````
The above will compile and run the tests.
