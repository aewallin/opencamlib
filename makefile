all: ocl

ocl: ocl.o point.o triangle.o stlsurf.o cutter.o
	g++ ocl.o point.o triangle.o stlsurf.o cutter.o -shared -o ocl.so  -lboost_python-mt  -lpython2.6

ocl.o: ocl.cpp ocl.h
	g++  -fPIC -o ocl.o -I/usr/include/python2.6 -c ocl.cpp 

point.o: point.cpp point.h
	g++  -fPIC -o point.o -c point.cpp 

triangle.o: triangle.cpp triangle.h
	g++  -fPIC -o triangle.o -c triangle.cpp 

stlsurf.o: stlsurf.cpp stlsurf.h
	g++  -fPIC -o stlsurf.o -c stlsurf.cpp 

cutter.o: cutter.cpp cutter.h
	g++  -fPIC -o cutter.o -c cutter.cpp 

doc: Doxyfile point.h triangle.h stlsurf.h cutter.h
	doxygen

pdf: doc
	cd ./doc/latex ; make pdf ; cd ../.. ; cp ./doc/latex/refman.pdf ./ocl.pdf

clean:
	rm -rf *.o *.so *.pyc *.pdf ; rm -Rf doc

# learn more from here:
# http://mrbook.org/tutorials/make/
# and here
# http://www.gnu.org/software/make/manual/make.html
