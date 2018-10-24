CFLAGS=-Wno-deprecated -Wall -ansi -pedantic -std=c++11 -Wfatal-errors
INCLUDES=-I ../log/ -I ../tools/class/
#The variable EXT_INCLUDES may be passed to this makefile when we need to specify the location of its dependencies.
LIBS=-lpthread
DEPS_MAIN=main.cpp obj/server.o obj/example_logic.o obj/client_writer.o
EXTERNAL_DEPS=../log/objects/log.o ../log/objects/log_tools.o ../tools/objects/arg_manager.o ../tools/objects/string_utils.o ../tools/objects/text_reader.o

clean:
	rm obj/*; rm ./a.out; rm logs/*;

all: dirs $(DEPS_MAIN)
	g++ $(CFLAGS) $(DEPS_MAIN) $(EXTERNAL_DEPS) $(INCLUDES) $(LIBS)

dirs:
	mkdir -p obj; mkdir -p logs;

obj/server.o: src/server.h src/server.cpp
	g++ $(CFLAGS) $(EXT_INCLUDES) $(INCLUDES) -c src/server.cpp -o obj/server.o

obj/example_logic.o: src/example_logic.h src/example_logic.cpp
	g++ $(CFLAGS) $(EXT_INCLUDES) $(INCLUDES) -c src/example_logic.cpp -o obj/example_logic.o

obj/client_writer.o: src/client_writer.h src/client_writer.cpp
	g++ $(CFLAGS) $(EXT_INCLUDES) $(INCLUDES) -c src/client_writer.cpp -o obj/client_writer.o
