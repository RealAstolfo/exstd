CXX = zig c++
INC = -I./include
LIB = -L/lib -lz

ZTARGET = -target native

CFLAGS = ${ZTARGET} -march=native -O3 -g -Wall -Wextra -pedantic $(INC)
CXXFLAGS = -std=c++20 $(CFLAGS)
LDFLAGS = $(LIB) -O3

all: docs open

docs:
	doxygen doxyfile

zstream-test:
	${CXX} ${CXXFLAGS} ${LIB} builds/test/zstream_test.cpp -o $@

open:
	firefox file:///home/dots/Documents/Projects/exstd/builds/docs/html/index.html

clean:
	-rm -rf builds/docs/*
	-rm zstream-test
