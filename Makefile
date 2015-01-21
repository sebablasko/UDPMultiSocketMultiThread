all: server client
	
all_profiling: serverProfiling client

server: server.o ../ssocket.o
	gcc server.o ../ssocket.o -o server -lpthread

serverProfiling: server.o ../ssocket.o
	gcc -fno-omit-frame-pointer -ggdb server.o ../ssocket.o -o server -lpthread

rm_server:
	rm server server.o

client: client.o ../ssocket.o
	gcc client.o ../ssocket.o -o client -lpthread

rm_client:
	rm client client.o

clean: rm_client rm_server