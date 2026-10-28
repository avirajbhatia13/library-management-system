CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
FLTK_CXX = $(shell fltk-config --cxxflags)
FLTK_LD  = $(shell fltk-config --ldflags)

CORE = src/Date.cpp src/LibraryItem.cpp src/Member.cpp \
       src/Transaction.cpp src/Library.cpp src/Report.cpp
GUI  = src/LibraryWindow.cpp src/main.cpp

all: build/library

build/library: $(CORE) $(GUI) include/*.h
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(FLTK_CXX) $(CORE) $(GUI) -o $@ $(FLTK_LD)

build/tests: $(CORE) tests/test_library.cpp include/*.h
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(CORE) tests/test_library.cpp -o $@

run: build/library
	./build/library

test: build/tests
	./build/tests

clean:
	rm -rf build

.PHONY: all run test clean
