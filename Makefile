CC=gcc
CFLAGS=-Wall -Wextra

all: server client 

server: server.c
	$(CC) $(CFLAGS) -o server server.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f server client *.o

help:
	@echo "Usage:"
	@echo "  make        - Build both server and client"
	@echo "  make server - Build only the server"
	@echo "  make client - Build only the client"
	@echo "  make clean  - Remove compiled programs"
	@echo ""
	@echo "Running the programs:"
	@echo "  ./server            - Start the server"
	@echo "  ./client hostname   - Connect to server at hostname"
	@echo ""
	@echo "Note: The server must be started before the client."