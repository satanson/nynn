vpath %.cc ../src ../test ../tool
vpath %.cpp ../src/ ../test ../tool
ifdef dbg
DEBUG:=-DDEBUG -g -ggdb3 -rdynamic
endif


BOOST_DIR = /usr/include/boost
THRIFT_DIR = /usr/local/include/thrift
LIB_DIR = /usr/local/lib

GEN_SRC = DataX.cpp DataXMaster.cpp DataXPeer.cpp\
		  nynn_mm_constants.cpp nynn_mm.cpp nynn_mm_types.cpp 

datax: dataxserv dataxceiv

dataxserv: DataXServer.cpp ${GEN_SRC}
	g++  --std=c++0x -o $@ -I${THRIFT_DIR} -I${BOOST_DIR} -I../src -I../inc -I. -L${LIB_DIR} $^ -lthrift -lthriftnb -levent

dataxceiv: DataXceiver.cpp ${GEN_SRC}
	g++  --std=c++0x -o $@ -I${THRIFT_DIR} -I${BOOST_DIR} -I../src -I../inc -I. -L${LIB_DIR} $^ -lthrift -lthriftnb -levent
creatchk:createChunk.cc
	g++ --std=c++0x -o $@ $^ -I../inc

clean:
	rm -fr `ls|grep -vi makefile`
