PROGRAM = game
CC      = gcc
CFLAGS  = -g -ansi -Wall -I/usr/X11R6/include -I/usr/pkg/include
LDFLAGS = -L/usr/X11R6/lib -L/usr/pkg/lib
LDLIBS  = -lglut -lGLU -lGL -lm

$(PROGRAM): game.o image.o
	$(CC) $(LDFLAGS) -o  $(PROGRAM) image.o game.o $(LDLIBS) 

image.o: image.c
	$(CC) -c image.c
	

.PHONY: clean 

clean:
	-rm *.o $(PROGRAM) 

