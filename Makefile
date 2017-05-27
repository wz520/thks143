# Makefile for mingw32

CFLAGS = -O2 -c -finput-charset=cp936 -fexec-charset=cp936
LFLAGS = -s
LIBS = -lcomdlg32
OBJS = main.o
CC = gcc
LD = ld
RM = del

thks143.exe: $(OBJS)
	$(CC) $(LFLAGS) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $<

main.o: main.c common.h

clean:
	@echo cleaning...
	-$(RM) main.o
	-$(RM) thks143.exe
	@echo cleaning complete.

.PHONY: clean
