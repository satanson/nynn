vpath %.cc ../src ../test
ifdef dbg
DEBUG:=-DDEBUG -g
endif
CXXFLAGS+= -I ../inc -L ../lib -lnynn -pthread -lrt -rdynamic $(DEBUG)

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
