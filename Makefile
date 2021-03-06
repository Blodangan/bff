CC = gcc
CFLAGS = -ansi -pedantic -Wall
CPPFLAGS = -D_XOPEN_SOURCE
INSTALL = install
bindir = /usr/local/bin

.PHONY: all
all: bff

.PHONY: install
install: all
	$(INSTALL) bff $(bindir)/bff

.PHONY: uninstall
uninstall:
	$(RM) $(bindir)/bff

.PHONY: clean
clean:
	$(RM) *.o bff
