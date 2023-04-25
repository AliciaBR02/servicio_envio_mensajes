CC = gcc
CFLAGS = -lrt
OBJS = servidor cliente
BIN_FILES = servidor cliente

all: $(OBJS)

servidor:  servidor.c operaciones/operaciones.c sockets/sockets.c
	$(CC)  $(CFLAGS) $^ -o $@.out

clean:
	rm -f $(BIN_FILES) *.out ./database/*.txt

re:	clean all

.PHONY: all servidor cliente clean re