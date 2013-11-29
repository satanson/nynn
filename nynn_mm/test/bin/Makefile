vpath %.cc ../src
ifdef dbg
DEBUG:=-DDEBUG -g -ggdb3 -rdynamic
endif
all=testMultiAccessSubgraph testAccessSubgraph formatSubgraph testSynchronizaton

all:${all}
testMultiAccessSubgraph:testMultiAccessSubgraph.cc
	g++ $(DEBUG) -o $@ $^  -I../../inc/ -I../inc/ -lpthread -lrt
testAccessSubgraph:testAccessSubgraph.cc
	g++ $(DEBUG) -o $@ $^  -I../../inc/ -I../inc/ -lpthread -lrt
formatSubgraph:formatSubgraph.cc
	g++ $(DEBUG) -o $@ $^  -I../../inc/ -I../inc/ -lpthread -lrt
testSynchronizaton:testSynchronization.cc
	g++ $(DEBUG) -o $@ $^  -I../../inc/ -I../inc/ -lpthread -lrt
.cc.o: 
	g++ $(DEBUG) -c $@ $^  -I../../inc/ -I../inc/ -lpthread -lrt

