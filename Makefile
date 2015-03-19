CXXFLAGS = --std=c++11 -pthread -Wall #-DFSELECT_THREAD_SAFE

test: test.o fselect.o
	$(CXX) -o $@ $^

clean:
	rm -rf *.o test doc
doc:
	doxygen

test.o: test.cc fselect.h fselect.o

fselect.o: fselect.cc fselect.h
