CC= gcc
CFLAGS= -Wall -Wextra -g

BINARIES= client.out server.out
SOURCES= client.c server.c utils.c cmd.c

OBJS=$(patsubst %.c, %.o, $(SOURCES))

all: $(BINARIES)

client.out: client.o utils.o
	$(CC) $(CFLAGS) $^ -o $@

server.out: server.o utils.o cmd.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c 
	$(CC) $(CFLAGS) -c $^

utils.c: utils.h
cmd.c: cmd.h

clean:
	rm -f *.o

distclean: clean
	rm -f $(BINARIES)

