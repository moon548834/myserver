CC= g++
TARGET= main
OBJECTS= main.o http_conn.o 
LDFLAGS=-lboost_system -lboost_filesystem -lpthread -lrt

$(TARGET): $(OBJECTS)
	$(CC) $^ $(LDFLAGS) -o $@
clean:
	rm $(OBJECTS) $(TARGET)
