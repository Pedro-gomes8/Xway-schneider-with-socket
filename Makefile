all: bin/main bin/gestionnaire


SRC=src/pcXway.cpp src/Tram.cpp src/Train.cpp src/SocketHandler.cpp

CFLAGS=-std=c++20 -stdlib=libc++
LDFLAGS=-lc++

IFLAGS=-I/opt/homebrew/include

bin/main: $(SRC) include/Tram.h
	clang $(CFLAGS) $(IFLAGS) $(SRC) -o $@ $(LDFLAGS)

bin/gestionnaire: src/gestionnaireNaive.cpp
	clang $(CFLAGS) $(IFLAGS) src/gestionnaireNaive.cpp -o $@ $(LDFLAGS)
clean:
	rm -rf bin/*