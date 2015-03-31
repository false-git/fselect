GMOCK_DIR=$(HOME)/prj/gmock-1.7.0
GTEST_DIR=$(GMOCK_DIR)/gtest

INCLUDES=-I$(GMOCK_DIR)/include -I$(GTEST_DIR)/include

CXXFLAGS = $(INCLUDES) -g --std=c++11 -pthread -Wall #-DFSELECT_THREAD_SAFE

gtest: gtest.o fselect.o select.o
	$(CXX) -o $@ $^ $(GMOCK_DIR)/src/gmock-all.o $(GTEST_DIR)/src/gtest-all.o

clean:
	rm -rf *.o gtest doc
doc:
	doxygen

gtest.o: gtest.cc

fselect.o: fselect.cc fselect.h

select.o: select.cc select.h

