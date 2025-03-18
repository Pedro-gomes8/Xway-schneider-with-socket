all: bin/main


#SRC=src/pcXway.cpp src/Tram.cpp src/gestionnaire.cpp
SRC=src/tram.cpp src/gestionnaire.cpp

CFLAGS=-std=c++11 #-stdlib=libc++
LDFLAGS=#-lc++

bin/main: $(SRC) include/tram.h include/mapping.h
	g++ $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

clean:
	rm -rf bin/*