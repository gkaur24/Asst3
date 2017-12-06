#CC = gcc
#CFLAGS = -Wall -g
#DFLAGS = -pthread

all: netfileserver.c client.c libnetfiles.c libnetfiles.h
        make client
        make server

client: client.c libnetfiles.c libnetfiles.h
        $(CC) $(CFLAGS) -o client client.c libnetfiles.c

server: netfileserver5.c libnetfiles.c libnetfiles.h
        $(CC) $(CFLAGS) -o server netfileserver.c libnetfiles.c $(DFLAGS)

clean:
        rm -f client
        rm -f server
        rm -rf server.dSYM
        rm -rf client.dSYM