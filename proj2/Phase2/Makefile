CC = gcc
CFLAGS = -Wall -Wextra

all: myShell

myShell: myshell.o
	$(CC) $(CFLAGS) -o myShell myshell.o

myshell.o: myshell.c myshell.h
	$(CC) $(CFLAGS) -c myshell.c

clean:
	rm -f myShell myshell.o