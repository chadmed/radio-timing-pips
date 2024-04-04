PREFIX ?= /usr
DATADIR ?= $(PREFIX)/share
BINDIR ?= $(PREFIX)/bin

EXTRA_CFLAGS ?=
CFLAGS= -Wall -Werror -lasound -lpthread -O2 -fstack-protector-strong $(EXTRA_CFLAGS)

build:
	$(CC) $(CFLAGS) pips.c -o pips

install:
	install -pm0644 pips $(DESTDIR)$(BINDIR)/timesignal

clean:
	rm -rf build/*

uninstall:
	rm -rf $(DESTDIR)$(BINDIR)/timesignal

all: build install
