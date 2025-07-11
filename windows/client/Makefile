CC = gcc
CFLAGS = -Wall
LDFLAGS = -lws2_32

SRCS = client.c

all: client.exe

client.exe: $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o *.exe
