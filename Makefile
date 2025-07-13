CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

all: tracker peer test_bencode compile_test

tracker: Tracker/Tracker/Tracker.cpp bencode.cpp
	$(CXX) $(CXXFLAGS) -o tracker Tracker/Tracker/Tracker.cpp bencode.cpp -lws2_32

peer: Peer/Peer/Peer.cpp bencode.cpp bt_protocol.cpp
	$(CXX) $(CXXFLAGS) -o peer Peer/Peer/Peer.cpp bencode.cpp bt_protocol.cpp -lws2_32

test_bencode: test_bencode.cpp bencode.cpp
	$(CXX) $(CXXFLAGS) -o test_bencode test_bencode.cpp bencode.cpp

compile_test: compile_test.cpp bencode.cpp
	$(CXX) $(CXXFLAGS) -o compile_test compile_test.cpp bencode.cpp

clean:
	rm -f tracker peer test_bencode compile_test

.PHONY: all clean
