all: bin/main


SRC=src/pcXway.c


bin/main: $(SRC)
	gcc $^ -o $@


clean:
	rm -rf bin/*