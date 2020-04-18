all: client server

client : client.c
	gcc client.c -o client

server : server.c
	gcc server.c -pthread -o server

clean:
	rm client server
