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

HEADERS = \
	$(CURDIR)/src/server/*.h     \
	$(CURDIR)/src/util/*.h       \
	$(CURDIR)/src/socks5/*.h     \
	$(CURDIR)/src/network/*.h    \

BINARY = $(CURDIR)/build/socks

clean:
	rm build/socks

build: $(BINARY)

$(BINARY): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $(BINARY) $(SOURCES)
