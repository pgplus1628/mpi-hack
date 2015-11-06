
CXX?=g++
CXXFLAGS?=-fPIC -O3 -g -Wall -std=c++11 -DNDEBUG
LDFLAGS?=-L/home/zork/loclib/mpich-3.1.4/lib -lglog -lmpich -lgflags
INFLAGS?=-I. -I/home/zork/loclib/mpich-3.1.4/include

all : main

main : main.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

main.o : main.cpp
	$(CXX) $(CXXFLAGS) $(INFLAGS) -c $^ -o $@

.PHONY:
clean :
	rm -f main
	rm -f main.o
