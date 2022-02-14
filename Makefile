default: build

CURDIR = $(shell pwd)

CC = gcc
CFLAGS = -I$(CURDIR)/src -g

SOURCES = \
	$(CURDIR)/src/server/*.c     \
	$(CURDIR)/src/util/*.c       \
	$(CURDIR)/src/socks5/*.c     \
	$(CURDIR)/src/network/*.c    \
	$(CURDIR)/src/*.c

clean:
	rm build/socks

build: socks

socks: $(SOURCES)
	$(CC) $(CFLAGS) -o $(CURDIR)/build/socks $(SOURCES)
