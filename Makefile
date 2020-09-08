CC = clang
ARGS = -Wall -g -pthread

all: server
	
	
server:
	$(CC) -o server $(ARGS) server.c 
	
	
clean:
	rm -rf server *.o