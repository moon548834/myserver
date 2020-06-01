CC=g++
CFLAGS=-Wall -std=c++11 -I/usr/local/include
LDFLAGS=-L/usr/local/lib -lboost_system -lboost_filesystem
SRCS=server.cpp connection.cpp 

OBJS=$(SRCS:.c=.o)

server:$(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ 

%.o:%.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf *.o
