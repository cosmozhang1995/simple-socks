default: build

CURDIR = $(shell pwd)

CC = gcc
CFLAGS = -I$(CURDIR)/src -g

SOURCES = $(CURDIR)/src/*.c $(CURDIR)/src/server/*.c $(CURDIR)/src/util/*.c

clean:
	rm build/socks

build: socks

socks: $(SOURCES)
	$(CC) $(CFLAGS) -o $(CURDIR)/build/socks $(SOURCES)
