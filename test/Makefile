TESTS=ghetto_list

CC=gcc
CFLAGS=-g -O0 -I../
LDFLAGS=-L../ -lghetto

$(TESTS): $(TESTS).o
	$(CC) $(LDFLAGS) -o $@ $@.o

.c.o:
	$(CC) $(CFLAGS) -c $<
