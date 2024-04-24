CXX = g++
CXXFLAGS = -std=c++17 -Wall

all: my_server

my_server: echo_server.o epoll_server.o
	$(CXX) $^ -o $@

echo_server.o: echo_server.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

epoll_server.o: epoll_server.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o my_server
