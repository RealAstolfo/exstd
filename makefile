CXX = g++
INC = -I./include
LIB = -L/lib -lz

CFLAGS = -march=native -O3 -g -Wall -Wextra -Wno-missing-field-initializers -pedantic $(INC)
CXXFLAGS = -std=c++20 -fsanitize=address $(CFLAGS)
LDFLAGS = $(LIB) -O3

all: docs open

docs:
	doxygen doxyfile

zstream-test:
	${CXX} ${CXXFLAGS} ${LIB} builds/test/zstream_test.cpp -o $@

open:
	qutebrowser file:///home/dots/Documents/Projects/exstd/builds/docs/html/index.html

clean:
	rm -rf builds/docs/*
	rm zstream-test
