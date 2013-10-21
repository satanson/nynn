vpath %.cc ../src ../test
ifdef dbg
DEBUG:=-DDEBUG -g -ggdb3 -rdynamic
endif
CXXFLAGS+= -std=c++0x -I ../inc -L ../lib -pthread -lrt -lnynn $(DEBUG)
testContext:testContext.o
	$(CXX) $(CXXFLAGS) -o $@ $^
testProcessOnce:testProcessOnce.o
	$(CXX) $(CXXFLAGS) -o $@ $^
testlocal:testlocal.o
	$(CXX) $(CXXFLAGS) -o $@ $^
testMapFile:testMapFile.o
	$(CXX) $(CXXFLAGS) -o $@ $^
testNynnError:testNynnError.o
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o:%.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $^

clean:
	rm -fr *.o
