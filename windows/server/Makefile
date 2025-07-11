CC = gcc
CFLAGS = -Wall
LDFLAGS = -lws2_32

SRCS = server.c lib.c
OBJS = $(SRCS:.c=.o)

all: server.exe

server.exe: $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o *.exe
