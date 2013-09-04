vpath %.cpp ../src ../test
ifdef dbg
DEBUG:=-DDEBUG -g
endif
CXXFLAGS+= -I ../inc -lpthread -lrt $(DEBUG)
nynn_daemon:nynn_daemon.o public.o socket.o nynn.o
	$(CXX) $(CXXFLAGS) -o $@ $^
aligndemo:aligndemo.o
	$(CXX) $(CXXFLAGS) -o $@ $^
nynn_serv:nynn_serv.o public.o socket.o
	$(CXX) $(CXXFLAGS) -o $@ $^
nynn_cli:nynn_cli.o boss.o public.o socket.o
	$(CXX) $(CXXFLAGS) -o $@ $^
boss_demo:boss_demo.o boss.o public.o
	$(CXX) $(CXXFLAGS) -o $@ $^
client: client.o public.o socket.o
	$(CXX) $(CXXFLAGS) -o $@ $^
log_demo:log_demo.o public.o
	$(CXX) $(CXXFLAGS) -o $@ $^
gethostaddr:gethostaddr.o public.o
	$(CXX) $(CXXFLAGS) -o $@ $^
text_demo:text_demo.o public.o
	$(CXX) $(CXXFLAGS) -o $@ $^
serv:serv.o public.o socket.o nynn.o
	$(CXX) $(CXXFLAGS) -o $@ $^
queue_demo:queue_demo.o public.o 
	$(CXX) $(CXXFLAGS) -o $@ $^
nynn_cli_read:nynn_cli_read.o public.o socket.o nynn.o
	$(CXX) $(CXXFLAGS) -o $@ $^
nynn_udp_read:nynn_udp_read.o public.o socket.o nynn.o
	$(CXX) $(CXXFLAGS) -o $@ $^
nynn_udp_write:nynn_udp_write.o public.o socket.o nynn.o
	$(CXX) $(CXXFLAGS) -o $@ $^
nynn_cli_write:nynn_cli_write.o public.o socket.o nynn.o
	$(CXX) $(CXXFLAGS) -o $@ $^
nynn_cli_demo:nynn_cli_demo.o public.o socket.o nynn.o
	$(CXX) $(CXXFLAGS) -o $@ $^
cli:cli.o public.o socket.o nynn.o
	$(CXX) $(CXXFLAGS) -o $@ $^
getopt_demo:getopt_demo.o
	$(CXX) $(CXXFLAGS) -o $@ $^
socket_demo:socket_demo.o public.o socket.o
	$(CXX) $(CXXFLAGS) -o $@ $^
%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

clean:
	rm -fr *.o
	rm -f boss_demo
	rm -f client
	rm -f socket_demo
	rm -f nynn_serv
	rm -f nynn_cli
	rm -f *~
	rm -f log_demo
	rm -f gethostaddr
