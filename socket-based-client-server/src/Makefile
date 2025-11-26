CC=gcc
CFLAGS=-lrt -lpthread
LDFLAGS=-L. -lclaves -Wl,-rpath,.

all: libclaves.so cliente servidor

default: all

cliente: app-cliente.c libclaves.so
	$(CC) -o cliente app-cliente.c $(LDFLAGS)

servidor: servidor-sock.c claves.c serial.c
	$(CC) -o servidor servidor-sock.c claves.c serial.c $(CFLAGS)

libclaves.so: proxy-sock.c serial.c
	$(CC) -fPIC -shared -o libclaves.so proxy-sock.c serial.c

clean:
	rm -f cliente servidor libclaves.so
