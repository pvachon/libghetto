OBJS = ghetto_fp.o ghetto_file.o

INCLUDES = -I. -Wall

CC = gcc

CFLAGS = $(INCLUDES)
LDFLAGS = -shared

TARGET = libghetto.so

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) $(OBJS) $(TARGET)

