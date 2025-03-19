all: bin/main

#SRC=src/pcXway.cpp src/Tram.cpp src/gestionnaire.cpp
SRC=src/tram.cpp src/gestionnaire.cpp src/gestionnaireJson.cpp
ICL=include/tram.h include/mapping.h

CFLAGS=-std=c++20 #-stdlib=libc++
LDFLAGS=#-lc++ #-ljson_spirit

test: bin/test

mainJson: bin/mainJson 

mainMulti: bin/mainMulti 

bin/mainMulti: src/gestionnaireMulti.cpp $(ICL)
	g++ $(CFLAGS) $< -o $@ $(LDFLAGS)

bin/mainJson: src/gestionnaireJson.cpp $(ICL)
	g++ $(CFLAGS) $< -o $@ $(LDFLAGS)

bin/main: $(SRC) $(ICL)
	g++ $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

bin/test: src/DummyClient.cpp
	g++ $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -rf bin/*