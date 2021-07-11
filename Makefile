CC=gcc
CFLAGS=-g

server: server.c query.c server.h
	$(CC) server.c query.c -o ../server/server.exe $(CFLAGS)

clean:
	rm ../server/server.exe