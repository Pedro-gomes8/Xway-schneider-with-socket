all: bin/main


SRC=src/pcXway.cpp src/Tram.cpp src/Train.cpp src/SocketHandler.cpp

CFLAGS=-std=c++11 -stdlib=libc++
LDFLAGS=-lc++

bin/main: $(SRC) include/Tram.h
	clang $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)


clean:
	rm -rf bin/*