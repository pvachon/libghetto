OBJS = ghetto_fp.o \
       ghetto_file.o \
       ghetto_ifd.o \
       ghetto_tag.o \
       ghetto_image.o

INCLUDES = -I. -Wall
DEFINES = -D_DEBUG

ENDIANESS = -DMACH_ENDIANESS=1

CC = gcc

CFLAGS = -O0 -g $(DEFINES) $(ENDIANESS) $(INCLUDES)
LDFLAGS = -shared

TARGET = libghetto.so

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) $(OBJS) $(TARGET)

