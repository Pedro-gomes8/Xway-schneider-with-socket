all: bin/main bin/gestionnaire

PORT:=8080
PORT2=$(shell expr $(PORT) + 1)
PORT3=$(shell expr $(PORT) + 2)
PORT4=$(shell expr $(PORT) + 3)

IP_CLIENT=172.31.68.121
IP_GESTIONNAIRE=172.31.68.121
IP_AUTOMATE=10.31.125.14

SRC=src/pcXway.cpp src/Tram.cpp src/Train.cpp src/SocketHandler.cpp src/ResponseRegistry.cpp src/Receiver.cpp src/Sender.cpp

CFLAGS=-std=c++20 #-stdlib=libc++
LDFLAGS=-lc++

IFLAGS=-I/opt/homebrew/include

bin/main: $(SRC) include/Tram.h
	g++ $(CFLAGS) $(IFLAGS) $(SRC) -o $@

bin/gestionnaire: src/gestionnaireNaive.cpp
	g++ $(CFLAGS) $(IFLAGS) src/gestionnaireNaive.cpp -o $@

gestionnaire:
	./bin/gestionnaire $(IP_GESTIONNAIRE) $(PORT)

Train1:
	./bin/main $(IP_CLIENT) $(IP_AUTOMATE) $(IP_CLIENT) $(PORT) 39 30 16
Train2:
	./bin/main $(IP_CLIENT) $(IP_AUTOMATE) $(IP_CLIENT) $(PORT2) 42 37 17
Train3:
	./bin/main $(IP_CLIENT) $(IP_AUTOMATE) $(IP_CLIENT) $(PORT3) 49 61 18
Train4:
	./bin/main $(IP_CLIENT) $(IP_AUTOMATE) $(IP_CLIENT) $(PORT4) 52 35 19

clean:
	rm -rf bin/*