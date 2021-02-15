# ctagd
ctagd is a tag based message passing interface over sockets (MPIS for short).

## Design
ctagd works by having a master program (also called a server) and communicates
with several slave programs (also called clients).

## Testing
````
$ make test
````
The above will compile and run the tests.
