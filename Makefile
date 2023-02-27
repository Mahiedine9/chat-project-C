CC = gcc
CFLAGS = -Wall -fsanitize=address
LDFLAGS = -fsanitize=address -lncurses

EXES = mtc_client mtc_server

all : $(EXES)

mtc_client.o : console.h protocol.h utils.h mtc_client.c

console.o : console.h

mtc_server.o : mtc_server.c protocol.h utils.h

mtc_client : mtc_client.o console.o
	$(CC) $(LDFLAGS) -o $@ $^

mtc_server : mtc_server.o
	$(CC) $(LDFLAGS) -o $@ $^

.o : .c

clean :
	rm -rf *.o
	rm -rf $(EXES)


cleanedit :
	rm *~

