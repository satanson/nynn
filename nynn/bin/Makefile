vpath %.cpp ../src ../test
ifdef dbg
DEBUG:=-DDEBUG -g
endif
CXXFLAGS+= -I ../inc -L ./  -lpthread -lrt -fPIC $(DEBUG)
basedir=/usr/nynn
all:nynn_daemon libnynn.so nynn_read nynn_write
	rm -fr *.o

libnynn.so:public.o nynn.o socket.o nynn.o nynn_daemon.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -shared 

nynn_daemon:nynn_daemon.o public.o socket.o nynn.o
	$(CXX) $(CXXFLAGS) -o $@ $^
nynn_write:nynn_write.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ -L$(basedir)/lib -lnynn
nynn_read:nynn_read.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ -L$(basedir)/lib -lnynn

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

install:
	cp nynn_daemon /usr/bin/
	mkdir -p $(basedir)/lib
	cp libnynn.so  $(basedir)/lib/
	mkdir -p $(basedir)/inc
	cp -r ../inc/*      $(basedir)/inc/
	echo $(basedir)/lib >/etc/ld.so.conf.d/nynn.conf
	ldconfig

uninst:
	rm -fr /usr/bin/nynn_daemon
	rm -fr $(basedir)/
	rm -fr /etc/ld.so.conf.d/nynn.conf


clean:
	rm -fr `ls|grep -vi makefile`
