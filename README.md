# ctagd
ctagd is a tag based message passing interface over sockets (MPIS for short).

## Design
ctagd works by having a master program (also called a server) and communicates
with several slave programs (also called clients).

## Building
To just build the object file, run:
````
$ make build
````
If you want to build the library, then run:
````
$ make lib
````

## Installing
TODO

## Testing
````
$ make test
````
The above will compile and run the tests.
