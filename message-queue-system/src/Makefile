CC=gcc
CFLAGS=-lrt -lpthread
LDFLAGS=-L. -lclaves -Wl,-rpath,.

all: libclaves.so cliente1 cliente2 servidor

default: all

cliente1: app-cliente-1.c libclaves.so
	$(CC) -o cliente1 app-cliente-1.c $(LDFLAGS)

cliente2: app-cliente-2.c libclaves.so
	$(CC) -o cliente2 app-cliente-2.c $(LDFLAGS) $(CFLAGS)

servidor: servidor-mq.c claves.c
	$(CC) -o servidor servidor-mq.c claves.c $(CFLAGS)

libclaves.so: proxy-mq.c
	$(CC) -fPIC -shared -o libclaves.so proxy-mq.c

clean:
	rm -f cliente1 cliente2 servidor libclaves.so
