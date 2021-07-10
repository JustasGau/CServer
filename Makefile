CC=gcc
CFLAGS=-g

server: server.c server.h
	$(CC) server.c -o ../server/server.exe $(CFLAGS)

clean:
	rm ../server/server.exe