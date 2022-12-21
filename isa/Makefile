# Makefile
#
# Project: Reader of news feed in format Atom & RSS with support of TLS
# Author: Matus Remen (xremen01@stud.fit.vutbr.cz)
# Date: 12.10.2022

PROG=feedreader
DIR=src

TEST=test
TDIR=tests

# == MacOS ==
export LDFLAGS="-L/usr/local/opt/openssl@3/lib"
export CPPFLAGS="-I/usr/local/opt/openssl@3/include"
# ===========

XMLLDFLAGS!=pkg-config --libs libxml-2.0
XMLLDFLAGS?=$(shell pkg-config --libs libxml-2.0)
XMLCFLAGS!=pkg-config --cflags libxml-2.0
XMLCFLAGS?=$(shell pkg-config --cflags libxml-2.0)

LDLIBS=$(XMLLDFLAGS) -lssl -lcrypto

CXX=g++
CXXFLAGS=-std=c++17 -Wall $(XMLCFLAGS) -static-libstdc++

.PHONY: all $(PROG) test pack clean

all: $(PROG)

# $^ doesn't work on eva.fit.vutbr.cz -> $(DIR)/*.o solves the issue
$(PROG): $(DIR)/$(PROG).o $(DIR)/arguments.o $(DIR)/client.o $(DIR)/parser.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) $(LDLIBS) -o $@ $(DIR)/*.o

$(TEST): $(TDIR)/$(TEST).o $(DIR)/arguments.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) $^ -o $@ $(LDLIBS)
	echo "Running tests..."
	./test

pack:
	zip -r xremen01.zip $(DIR) include/ $(TDIR) docs/manual.pdf Makefile Readme.md

clean:
	rm $(PROG) $(TEST) $(DIR)/*.o $(TDIR)/*.o
