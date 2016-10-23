CC=gcc
CFLAGS=-I -Wall -lrt -pthread
DEPS = cmds.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

shell: shell.o
	$(CC) -o shell shell.o $(CFLAGS)

clean:
	rm -f *.o vgcore* core* *~ shell