# $Id: Makefile,v 1.1.1.1 2008/05/15 13:49:25 xach Exp $

CFLAGS=-g

OBJECTS=sources.o card.o compositor.o util.o wfcomp.o jpeg.o png.o args.o

wfcomp: $(OBJECTS)
	gcc $(CFLAGS) -o wfcomp -ljpeg -lpng $(OBJECTS)

clean:
	rm -f wfcomp *.o *~
