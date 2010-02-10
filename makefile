all: camala

camala: camala.o point.o triangle.o stlsurf.o cutter.o
	g++ camala.o point.o triangle.o stlsurf.o cutter.o -shared -o camala.so  -lboost_python-mt  -lpython2.6

camala.o: camala.cpp camala.h
	g++  -fPIC -o camala.o -I/usr/include/python2.6 -c camala.cpp 

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

pdf:
	cd ./doc/latex ; pwd ; make pdf ; cd ../.. ; cp ./doc/latex/refman.pdf ./camala.pdf

clean:
	rm -rf *.o *.so

# learn more from here:
# http://mrbook.org/tutorials/make/
# and here
# http://www.gnu.org/software/make/manual/make.html
