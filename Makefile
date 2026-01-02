# Makefile for Online Voting System

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
LDFLAGS = -pthread

# Windows specific
ifeq ($(OS),Windows_NT)
    LDFLAGS += -lws2_32
endif

TARGETS = server test_voter_load

all: $(TARGETS)

server: server_main.cpp config.h voter.h candidate.h voting_manager.h disk_manager.h
	$(CXX) $(CXXFLAGS) -o server server_main.cpp $(LDFLAGS)

test_voter_load: test_voter_load.cpp config.h voter.h
	$(CXX) $(CXXFLAGS) -o test_voter_load test_voter_load.cpp

clean:
	rm -f $(TARGETS) *.o

run_server: server
	./server

run_test: test_voter_load
	./test_voter_load

.PHONY: all clean run_server run_test