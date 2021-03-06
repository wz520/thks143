# Makefile for mingw32

CFLAGS = -O2 -c -finput-charset=cp936 -fexec-charset=cp936
LFLAGS = -s
OBJS = main.o

thks143: $(OBJS)
	$(CC) $(LFLAGS) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $<

main.o: main.c common.h

clean:
	@echo cleaning...
	-$(RM) main.o
	-$(RM) thks143
	@echo cleaning complete.

.PHONY: clean
