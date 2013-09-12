vpath %.cc ../src ../test
ifdef dbg
DEBUG:=-DDEBUG -g
endif
CXXFLAGS+= -I ../inc -pthread -lrt -rdynamic $(DEBUG)
nynn_mm:public.o
	$(CXX) $(CXXFLAGS) -o $@ $^
testNynnError:testNynnError.o
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o:%.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $^

clean:
	rm -fr *.o
