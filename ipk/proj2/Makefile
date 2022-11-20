FILE = packet_sniffer
PROG = ipk-sniffer
CXX = g++
CXXFLAGS = -std=c++17
LIBS = -lpcap

.PHONY: all clean pack

all: $(PROG)

$(PROG): $(FILE).cpp
	$(CXX) $(CXXFLAGS) $(FILE).cpp -o $(PROG) $(LIBS)

pack:
	tar -cf xremen01.tar $(FILE).cpp Makefile README.md

clean:
	rm $(PROG)
