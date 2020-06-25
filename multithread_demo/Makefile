CC= g++
TARGET= main
OBJECTS= main.o http_conn.o 
LDFLAGS=-lboost_system -lboost_filesystem -lpthread -lrt -g

$(TARGET): $(OBJECTS)
	$(CC) $^ $(LDFLAGS) -o $@
%.o:%.cpp
	$(CC) -c -g $^
clean:
	rm $(OBJECTS) $(TARGET)
