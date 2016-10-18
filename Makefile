CC=gcc
CFLAGS=-I -Wall
DEPS = shell.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

shellmake: shellmake.o shell.o
	$(CC) -o shell shellmake.o shell.o $(CFLAGS)

clean:
	rm -f *.o core* *~ shell