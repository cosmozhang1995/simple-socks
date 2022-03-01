default: build

CURDIR = $(shell pwd)

CC = gcc
CFLAGS = -I$(CURDIR)/src -g

HEADERS = \
	$(CURDIR)/src/server/*.h     \
	$(CURDIR)/src/util/*.h       \
	$(CURDIR)/src/socks5/*.h     \
	$(CURDIR)/src/network/*.h    \
	$(CURDIR)/src/service/*.h    \
	$(CURDIR)/src/testing/*.h    \

SOURCES = \
	$(CURDIR)/src/server/*.c     \
	$(CURDIR)/src/util/*.c       \
	$(CURDIR)/src/socks5/*.c     \
	$(CURDIR)/src/network/*.c    \
	$(CURDIR)/src/service/*.c    \
	$(CURDIR)/src/testing/*.c    \

MAIN = $(CURDIR)/src/main.c

BINARY = $(CURDIR)/build/socks

clean:
	rm build/socks

build: $(BINARY)

$(BINARY): $(MAIN) $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $(BINARY) $(MAIN) $(SOURCES)



UT_SOURCES = $(SOURCES) $(CURDIR)/test/*.c
UT_HEADERS = $(HEADERS) $(CURDIR)/test/*.h
UT_CFLAGS = -I$(CURDIR)/test $(CFLAGS)

