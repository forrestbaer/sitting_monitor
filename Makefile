CC = gcc
CFLAGS = -Wall
LIBS = -lwiringPi

%: %.c
	$(CC) -o $@ $< $(LIBS) $(CFLAGS)

sitmond.o: sitmond.c

sitstat.o:
	$(CC) -o sitstat sitstat.c
