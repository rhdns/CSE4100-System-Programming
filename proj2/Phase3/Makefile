CC = gcc
CFLAGS = -Wall -Wextra
LIBS = -pthread -lrt 

all: myShell

myShell: myshell.o csapp.o
	$(CC) $(CFLAGS) -o myShell myshell.o csapp.o $(LIBS)

myshell.o: myshell.c myshell.h
	$(CC) $(CFLAGS) -c myshell.c

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

run: myShell
	./myShell

clean:
	rm -f myShell myshell.o csapp.o