LDFLAGS=-lboost_system -lboost_filesystem
CC=g++
CFLAGS=-Wall -std=c++11 
LDFLAGS=-lboost_system -lboost_filesystem
SRCS=server.cpp connection.cpp 

OBJS=$(SRCS:.cpp=.o)

server:$(OBJS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@ 

%.o:%.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf *.o server
