all: docs open

docs:
	doxygen doxyfile

open:
	qutebrowser file:///home/dots/Documents/Projects/exstd/builds/docs/html/index.html

clean:
	rm -rf builds/docs/*
