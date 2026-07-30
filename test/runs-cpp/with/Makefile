CXX ?= g++
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic -Iinclude
PORT ?= 8080

LIB_SOURCES := src/quadratic.cc src/plot.cc src/json_response.cc \
               src/http_server.cc src/static_files.cc
SERVER_SOURCES := $(LIB_SOURCES) src/main.cc
TEST_SOURCES := src/quadratic.cc src/plot.cc src/http_server.cc \
                tests/quadratic_test.cc

LIB_OBJECTS := $(LIB_SOURCES:.cc=.o)
SERVER_OBJECTS := $(SERVER_SOURCES:.cc=.o)
TEST_OBJECTS := $(TEST_SOURCES:.cc=.o)

.PHONY: all test run clean

all: quadcalc

quadcalc: $(SERVER_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

quadcalc_test: $(TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

test: quadcalc_test
	./quadcalc_test

run: quadcalc
	./quadcalc $(PORT) web

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f quadcalc quadcalc_test $(SERVER_OBJECTS) $(TEST_OBJECTS)
