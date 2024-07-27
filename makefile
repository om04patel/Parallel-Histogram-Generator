CC=gcc
CFLAGS=-Wall -g -std=c11 -D_POSIX_C_SOURCE=200809L
SOURCE=histogram_generator.c
EXEC=histogram_generator

all: $(EXEC)

$(EXEC): $(SOURCE)
	$(CC) $(CFLAGS) $(SOURCE) -o $(EXEC)

clean:
	rm -f $(EXEC) *~ core
