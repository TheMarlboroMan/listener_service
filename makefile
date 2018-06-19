CFLAGS=--std=c++11 -I ../terminaltools/src/
LIBS=-lpthread
DEPS_MAIN=main.cpp obj/server.o obj/log.o obj/logtools.o

clean:
	rm obj/*; rm ./a.out; rm logs/*;

all: $(DEPS_MAIN)
	g++ $(CFLAGS) $(DEPS_MAIN) $(LIBS)

obj/log.o: src/log.h src/log.cpp
	g++ $(CFLAGS) -c src/log.cpp -o obj/log.o

obj/logtools.o: src/logtools.h src/logtools.cpp
	g++ $(CFLAGS) -c src/logtools.cpp -o obj/logtools.o

obj/server.o: src/server.h src/server.cpp
	g++ $(CFLAGS) -c src/server.cpp -o obj/server.o
