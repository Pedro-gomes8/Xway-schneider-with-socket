#===================== IPs AND PORTs DECLARATION ===================


PORT:=8080
PORT2=$(shell expr $(PORT) + 1)
PORT3=$(shell expr $(PORT) + 2)
PORT4=$(shell expr $(PORT) + 3)

IP_CLIENT=172.31.68.121
IP_GESTIONNAIRE=172.31.68.121
IP_AUTOMATE=10.31.125.14

#===================== DIRECTORY DECLARATION ===================

SRC=src/pcXway.cpp src/Tram.cpp src/Train.cpp src/SocketHandler.cpp

#===================== FLAGS DECLARATION ===================

CFLAGS=-std=c++20 #-stdlib=libc++
LDFLAGS=-lc++

IFLAGS=-I/opt/homebrew/include

#=========================================================

all: bin/main bin/gestionnaireNaive

bin/main: $(SRC) include/Tram.h
	g++ $(CFLAGS) $(SRC) -o $@ 

bin/gestionnaire: src/gestionnaire.cpp
	g++ $(CFLAGS) src/gestionnaire.cpp -o $@

bin/gestionnaireNaive: src/gestionnaireNaive.cpp
	g++ $(CFLAGS) src/gestionnaireNaive.cpp -o $@ 

gestionnaireNaive:
	./bin/gestionnaireNaive $(IP_GESTIONNAIRE) $(PORT)

Train1:
	./bin/main $(IP_CLIENT) $(IP_AUTOMATE) $(IP_GESTIONNAIRE) $(PORT) 39 30 16
Train2:
	./bin/main $(IP_CLIENT) $(IP_AUTOMATE) $(IP_GESTIONNAIRE) $(PORT2) 42 37 17
Train3:
	./bin/main $(IP_CLIENT) $(IP_AUTOMATE) $(IP_GESTIONNAIRE) $(PORT3) 49 61 18
Train4:
	./bin/main $(IP_CLIENT) $(IP_AUTOMATE) $(IP_GESTIONNAIRE) $(PORT4) 52 35 19

PC1:
#	./bin/main $(IP_CLIENT) $(IP_AUTOMATE) $(IP_CLIENT) $(PORT) 39 30 16

PC2:
#	./bin/main $(IP_CLIENT) $(IP_AUTOMATE) $(IP_CLIENT) $(PORT2) 42 37 17

clean:
	rm -rf bin/*