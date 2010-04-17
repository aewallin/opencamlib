
# if the user doesn't say, install into /usr/local by default
PREFIX ?= /usr/local


PROGS =                       \
    drop_cutter_tst_1.py      \
    drop_cutter_tst_2.py      \
    drop_cutter_tst_3.py      \
    drop_cutter_tst_4.py      \
    drop_cutter_tst_5.py      \
    kdtree_movie1.py          \
    kdtree_movie2.py          \
    kdtree_tst_1.py           \
    kdtree_tst.py             \
    ocl_tst.py                \
    pfinish_tst_1.py          \
    stl2ocl_tst.py            \
    stlsurf_tst.py

STLS =                        \
    demo.stl                  \
    gnu_tux_mod.stl           \
    sphere.stl

# compiler
CC=g++
# compiler options
CFLAGS=-fPIC `python-config --includes` -Wall -c
# link options
LFLAGS=-shared -o ocl.so -Wl,-no-undefined -lboost_python-mt  `python-config --ldflags`


.PHONY: all
all: ocl.so

ocl.so: ocl.o point.o triangle.o stlsurf.o cutter.o cylcutter.o ballcutter.o bullcutter.o numeric.o kdtree.o pfinish.o arc.o line.o path.o pathfinish.o oellipse.o
	$(CC) $(LFLAGS) ocl.o point.o triangle.o stlsurf.o cutter.o cylcutter.o ballcutter.o bullcutter.o numeric.o kdtree.o pfinish.o arc.o line.o path.o pathfinish.o oellipse.o

ocl.o: ocl.cpp ocl.h
	$(CC)  $(CFLAGS) ocl.cpp 

point.o: point.cpp point.h
	$(CC)  $(CFLAGS) point.cpp 

triangle.o: triangle.cpp triangle.h
	$(CC)  $(CFLAGS) triangle.cpp 

stlsurf.o: stlsurf.cpp stlsurf.h
	$(CC)  $(CFLAGS) stlsurf.cpp 

cutter.o: cutter.cpp cutter.h
	$(CC)  $(CFLAGS) cutter.cpp 

cylcutter.o: cylcutter.cpp cutter.h
	$(CC)  $(CFLAGS) cylcutter.cpp 

ballcutter.o: ballcutter.cpp cutter.h
	$(CC)  $(CFLAGS) ballcutter.cpp 

bullcutter.o: bullcutter.cpp cutter.h
	$(CC)  $(CFLAGS) bullcutter.cpp 

numeric.o: numeric.h numeric.cpp
	$(CC)  $(CFLAGS) numeric.cpp

kdtree.o: kdtree.h kdtree.cpp
	$(CC)  $(CFLAGS) kdtree.cpp

pfinish.o: pfinish.h pfinish.cpp
	$(CC)  $(CFLAGS) pfinish.cpp

arc.o: arc.h arc.cpp
	$(CC)  $(CFLAGS) arc.cpp

line.o: line.h line.cpp
	$(CC)  $(CFLAGS) line.cpp

path.o: path.h path.cpp
	$(CC)  $(CFLAGS) path.cpp

pathfinish.o: pathfinish.h pathfinish.cpp
	$(CC)  $(CFLAGS) pathfinish.cpp

oellipse.o: oellipse.h oellipse.cpp
	$(CC)  $(CFLAGS) oellipse.cpp

.PHONY: install
install: ocl.so
	strip $^

	mkdir -p $(DESTDIR)$(PREFIX)/lib/python`python -c "import sys; print sys.version[:3]"`/dist-packages/
	install -m 0644 $^ $(DESTDIR)$(PREFIX)/lib/python`python -c "import sys; print sys.version[:3]"`/dist-packages/
	install -m 0644 camvtk.py $(DESTDIR)$(PREFIX)/lib/python`python -c "import sys; print sys.version[:3]"`/dist-packages/

	mkdir -p $(DESTDIR)$(PREFIX)/share/doc/python-opencam/examples
	install -m 0644 $(PROGS) $(DESTDIR)$(PREFIX)/share/doc/python-opencam/examples
	install -m 0644 $(STLS) $(DESTDIR)$(PREFIX)/share/doc/python-opencam/examples

.PHONY: doc
doc: Doxyfile manual.h point.h triangle.h stlsurf.h cutter.h numeric.h kdtree.h arc.h line.h path.h pathfinish.h
	doxygen

.PHONY: pdf
pdf: doc
	cd ./doc/latex ; make pdf ; cd ../.. ; cp ./doc/latex/refman.pdf ./ocl.pdf

.PHONY: clean
clean:
	rm -rf *.o *.so *.pyc *.pdf ; rm -Rf doc

# learn more from here:
# http://mrbook.org/tutorials/make/
# and here
# http://www.gnu.org/software/make/manual/make.html
