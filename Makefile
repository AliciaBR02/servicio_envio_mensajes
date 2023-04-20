CC = gcc
CFLAGS = -lrt
OBJS = servidor cliente
BIN_FILES = servidor cliente

all: $(OBJS)

servidor:  servidor.c 
	$(CC)  $(CFLAGS) $^ -o $@.out

cliente: cliente.c comunicacion.c sockets.c
	$(CC) -I $(CFLAGS) $^ -o $@.out

clean:
	rm -f $(BIN_FILES) *.out *.o *.so

re:	clean all

.PHONY: all servidor cliente clean re