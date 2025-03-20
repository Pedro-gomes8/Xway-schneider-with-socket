all: bin/main bin/gestionnaire


SRC=src/pcXway.cpp src/Tram.cpp src/Train.cpp src/SocketHandler.cpp

CFLAGS=-std=c++20 #-stdlib=libc++
LDFLAGS=-lc++

IFLAGS=-I/opt/homebrew/include

bin/main: $(SRC) include/Tram.h
	g++ $(CFLAGS) $(SRC) -o $@ 

bin/gestionnaire: src/gestionnaireNaive.cpp
	g++ $(CFLAGS) src/gestionnaireNaive.cpp -o $@ 
clean:
	rm -rf bin/*