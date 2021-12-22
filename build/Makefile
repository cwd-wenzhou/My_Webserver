CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 
PREFIX=	/usr/local
TARGET = server
OBJS =  ../code/log/*.cpp ../code/sql/*.cpp ../code/timer/*.cpp \
       ../code/http/*.cpp ../code/server/*.cpp \
       ../code/buffer/*.cpp ../code/main.cpp

server: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../bin/server  -pthread -lmysqlclient

install: server
	install -s server $(DESTDIR)$(PREFIX)/bin	

	
clean:
	rm -rf *.o




