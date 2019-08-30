.PHONY:all

all:server

server:server.cpp
	g++ $^ -o $@ -std=c++11 -L /usr/lib64/mysql -lmysqlclient -ljsoncpp -lpthread -lcrypto -g

.PHONY:clean
clean:
	rm server

