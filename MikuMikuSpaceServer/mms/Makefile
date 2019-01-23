CC = gcc
CXX = g++
LD = g++

CXXFLAGS = -g -ggdb -Wall -std=c++11 -I/usr/include/boost -I/usr/include/cryptopp -I/usr/include/mysqlcppconn
LIBS = -lboost_system -lboost_thread -lboost_date_time -lboost_filesystem -lboost_regex -lcryptopp -lpthread -lmysqlcppconn
LIBS_DIR = -L/usr/lib -L/usr/lib/x86_64-linux-gnu/ -L/usr/lib/mysqlcppconn

TARGET = Server

OBJS = $(patsubst %.cpp,%.o,$(wildcard *.cpp))

all: $(OBJS)
	$(LD) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS) $(LIBS_DIR)

clean:
	@rm -f $(OBJS) $(TARGET)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<
