#
# $Id: Makefile.noimake,v 1.1 1994/09/10 05:33:29 risto Exp risto $
#
# Compiler flags and include and library paths
# Change these variables as necessary for your system

CC = gcc -ansi
CFLAGS = -O
INCLUDES = -I. -I/usr/local/X11/include
LIBS = -L/usr/local/X11/lib -lXaw -lXmu -lXt -lXext -lX11 -lm

# Some common definitions...
RM = rm -f

# Rule to create .o files from .c files
.c.o:
	$(RM) $@
	$(CC) -c  $(CFLAGS) $(INCLUDES) $*.c

# Targets...
all:: dislex

main.o nets.o stats.o graph.o: globals.c defs.h prototypes.h

dislex: main.o nets.o stats.o graph.o gwin.o
	$(RM) $@
	$(CC) -o $@ $(CFLAGS) main.o nets.o stats.o graph.o gwin.o $(LIBS)

clean: 
	$(RM) *.o dislex
