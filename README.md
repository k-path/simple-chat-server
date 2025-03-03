# Simple Chat Server
Simple Chat Server is a stream socket based chat server where a client connects to a running server and sends 
data back and forth.

To build the executables run:
```
make # both at once
# or
make server
# or
make client
```

To see instructions:
```
make help
```

Run the executables:
```
./server
# then
./client hostname # like localhost
```

Improvements: 
- Use poll for synchronous I/O multiplexing (ability to monitor a bunch of sockets at once and then handle the ones that have data ready). 
