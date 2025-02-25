CC=gcc
CFLAGS=-Wall -Wextra

all: server client # need to modify so client takes a hostname

server: server.c
	$(CC) $(CFLAGS) -o server server.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f server client