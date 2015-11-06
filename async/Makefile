
CXX?=g++
#CXX=/home/zork/loclib/openmpi-1.10.1/bin/mpic++
CXXFLAGS?=-fPIC -O3 -g -Wall -std=c++11 -DNDEBUG
LDFLAGS?=-L/home/zork/loclib/mpich-3.1.4/lib -lglog -lmpich -lgflags
INFLAGS?=-I. -I/home/zork/loclib/mpich-3.1.4/include

#LDFLAGS?=-L/home/zork/loclib/openmpi-1.10.1/lib -lglog -lmpi -lgflags
#INFLAGS?=-I. -I/home/zork/loclib/openmpi-1.10.1/include


all : main

main : main.o basic_types.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

main.o : main.cpp
	$(CXX) $(CXXFLAGS) $(INFLAGS) -c $^ -o $@

basic_types.o : basic_types.cpp
	$(CXX) $(CXXFLAGS) $(INFLAGS) -c $^ -o $@


.PHONY:
clean :
	rm -f main
	rm -f *.o
