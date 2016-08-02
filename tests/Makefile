CXXLD= -pthread -L/usr/local/lib -lzmq 
CXXFLAGS= -c -Wall -g -O0 -I /usr/local/include
CXX=g++ -std=c++11

SRC:=\
	router.cpp

OBJ=$(patsubst %.cpp, %.o , $(SRC) )

all: app 
	
app: ${OBJ}
	${CXX} ${CXXLD} ${OBJ} -o app

%.o: %.cpp
	${CXX} ${CXXFLAGS} $< -o $@

valgrind:
	valgrind \
		--tool=memcheck \
		--leak-check=full \
		--track-fds=yes \
		--suppressions=valgrind.supp \
		./app

clean: 
	rm -f router.o app
