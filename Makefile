default: build

CXX = g++
SOURCES = src/*.cpp

clean:
	rm build/socks

build: socks

socks:
	$(CXX) -o build/socks $(SOURCES)
