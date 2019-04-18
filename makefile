ifneq ($(MAKECMDGOALS),clean)
ifndef EXT_INCLUDES
$(error EXT_INCLUDES is not set, please, set it to the location of the 'log' and 'tools' source files like 'make #target# EXT_INCLUDES="-I ../log -I ../tools"')
endif

ifndef EXT_LINK
$(error EXT_LINK is not set, please, set it to the location of the 'log' and 'tools' libraries like 'make #target# EXT_LINK="-L ../log -L ../tools"')
endif
endif

CFLAGS=-Wno-deprecated -Wall -ansi -pedantic -std=c++11 -Wfatal-errors
DEPS_MAIN=obj/server.o obj/client_reader.o obj/client_writer.o obj/openssl_wrapper.o
DEPS_EXAMPLE=example/obj/example_logic.o

ifdef DEBUG
CFLAGS+= -g
endif


ifdef WITH_SSL
CFLAGS+= -DWITH_SSL
SSL_LINK=-lssl -lcrypto
endif

ifdef WITH_SSL_CURRENT
CFLAGS+= -DWITH_SSL_CURRENT
endif

all: dirs example
	echo "All done";

clean:
	if [ -f example/a.out ]; then rm example/a.out; fi;\
	if [ -d obj ]; then rm -rf obj; fi;\
	if [ -d example/obj ]; then rm -rf example/obj; fi;\
	if [ -d example/logs ]; then rm -rf example/logs/*; fi;\

dirs:
	mkdir -p obj;\
	mkdir -p example/logs;\
	mkdir -p example/obj;\

obj/server.o: src/server.h src/server.cpp
	g++ $(CFLAGS) $(EXT_INCLUDES) -c src/server.cpp -o obj/server.o

obj/client_writer.o: src/client_writer.h src/client_writer.cpp
	g++ $(CFLAGS) $(EXT_INCLUDES) -c src/client_writer.cpp -o obj/client_writer.o

obj/client_reader.o: src/client_reader.h src/client_reader.cpp
	g++ $(CFLAGS) $(EXT_INCLUDES) -c src/client_reader.cpp -o obj/client_reader.o

#ifdef WITH_SSL
obj/openssl_wrapper.o: src/openssl_wrapper.h src/openssl_wrapper.cpp
	g++ $(CFLAGS) $(EXT_INCLUDES) -c src/openssl_wrapper.cpp -o obj/openssl_wrapper.o
#endif

example: $(DEPS_MAIN) $(DEPS_EXAMPLE) example/main.cpp
	g++ example/main.cpp $(CFLAGS) $(DEPS_MAIN) $(DEPS_EXAMPLE) $(EXT_INCLUDES) $(EXT_LINK) $(SSL_LINK) -llog -ltools

example/obj/example_logic.o: example/src/example_logic.h example/src/example_logic.cpp
	g++ $(CFLAGS) $(EXT_INCLUDES) -c example/src/example_logic.cpp -o example/obj/example_logic.o
