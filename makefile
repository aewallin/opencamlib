
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


.PHONY: all
all: ocl.so

ocl.so: ocl.o point.o triangle.o stlsurf.o cutter.o cylcutter.o ballcutter.o bullcutter.o numeric.o kdtree.o pfinish.o
	g++ ocl.o point.o triangle.o stlsurf.o cutter.o cylcutter.o ballcutter.o bullcutter.o numeric.o kdtree.o pfinish.o -shared -o ocl.so -Wl,-no-undefined -lboost_python-mt  -lpython2.6

ocl.o: ocl.cpp ocl.h
	g++  -fPIC -o ocl.o -I/usr/include/python2.6 -c ocl.cpp 

point.o: point.cpp point.h
	g++  -fPIC -o point.o -I/usr/include/python2.6 -c point.cpp 

triangle.o: triangle.cpp triangle.h
	g++  -fPIC -o triangle.o -I/usr/include/python2.6 -c triangle.cpp 

stlsurf.o: stlsurf.cpp stlsurf.h
	g++  -fPIC -o stlsurf.o -I/usr/include/python2.6 -c stlsurf.cpp 

cutter.o: cutter.cpp cutter.h
	g++  -fPIC -o cutter.o -I/usr/include/python2.6 -c cutter.cpp 

cylcutter.o: cylcutter.cpp cutter.h
	g++  -fPIC -o cylcutter.o -I/usr/include/python2.6 -c cylcutter.cpp 

ballcutter.o: ballcutter.cpp cutter.h
	g++  -fPIC -o ballcutter.o -I/usr/include/python2.6 -c ballcutter.cpp 

bullcutter.o: bullcutter.cpp cutter.h
	g++  -fPIC -o bullcutter.o -I/usr/include/python2.6 -c bullcutter.cpp 

numeric.o: numeric.h numeric.cpp
	g++  -fPIC -o numeric.o -I/usr/include/python2.6 -c numeric.cpp

kdtree.o: kdtree.h kdtree.cpp
	g++  -fPIC -o kdtree.o -I/usr/include/python2.6 -c kdtree.cpp

pfinish.o: pfinish.h pfinish.cpp
	g++  -fPIC -o pfinish.o -I/usr/include/python2.6 -c pfinish.cpp

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
doc: Doxyfile manual.h point.h triangle.h stlsurf.h cutter.h numeric.h kdtree.h
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
