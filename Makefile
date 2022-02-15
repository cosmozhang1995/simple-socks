default: build

CURDIR = $(shell pwd)

CC = gcc
CFLAGS = -I$(CURDIR)/src -g

HEADERS = \
	$(CURDIR)/src/server/*.h     \
	$(CURDIR)/src/util/*.h       \
	$(CURDIR)/src/socks5/*.h     \
	$(CURDIR)/src/network/*.h    \
	$(CURDIR)/src/testing/*.h    \

SOURCES = \
	$(CURDIR)/src/server/*.c     \
	$(CURDIR)/src/util/*.c       \
	$(CURDIR)/src/socks5/*.c     \
	$(CURDIR)/src/network/*.c    \
	$(CURDIR)/src/testing/*.c    \
	$(CURDIR)/src/*.c

BINARY = $(CURDIR)/build/socks

clean:
	rm build/socks

build: $(BINARY)

$(BINARY): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $(BINARY) $(SOURCES)
