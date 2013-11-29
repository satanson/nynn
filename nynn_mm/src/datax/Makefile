vpath %.cc ../src ../test
ifdef dbg
DEBUG:=-DDEBUG -g -ggdb3 -rdynamic
endif
nynndir=/usr/nynn
CXXFLAGS+= -std=c++0x -I ../inc -I $(nynndir)/inc -L $(nynndir)/lib -pthread -lrt -lnynn $(DEBUG) -fPIC
testloadconfig:testloadconfig.o
	$(CXX) $(CXXFLAGS) -o $@ $^
testgetarraylist:testgetarraylist.o
	$(CXX) $(CXXFLAGS) -o $@ $^
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
	rm -fr `ls|grep -vi makefile`

BOOST_DIR = /usr/include/boost
THRIFT_DIR = /usr/local/include/thrift
LIB_DIR = /usr/local/lib

GEN_SRC = ./DataXceiver.cpp ./DataX.cpp ./DataXMaster.cpp ./DataXPeer.cpp\
		  ./DataXServer.cpp ./nynn_mm_constants.cpp ./nynn_mm.cpp ./nynn_mm_types.cpp 

datax: dataxserv dataxceiv

dataxserv: DataXServer.cpp
	g++  --std=c++0x -o $@ -I${THRIFT_DIR} -I${BOOST_DIR}  -I. -L${LIB_DIR} $^ ${GEN_SRC}  -lthrift -lthriftnb -levent

dataxceiv: DataXceiver.cpp
	g++  --std=c++0x -o $@ -I${THRIFT_DIR} -I${BOOST_DIR}  -I. -L${LIB_DIR} $^ ${GEN_SRC}  -lthrift -lthriftnb -levent

clean:
	$(RM) -r dataxserv dataxceiv 
