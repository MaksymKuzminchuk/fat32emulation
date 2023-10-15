CC=gcc
LIBS = -lm

all: fat32emulation

fat32emulation: fat32emulation.c
	$(CC) fat32emulation.c $(LIBS) -o fat32emulation

clean:
	rm -rf *.o fat32emulation
