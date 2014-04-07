CXX=g++
CPPFLAGS= -I/usr/local/include/gnuradio -I/usr/local/include -march=native -Ofast
LDFLAGS= -lvolk -luhd -lboost_thread -lboost_program_options -lgnuradio-core -L/usr/local/lib -lpthread -lgruel 
OBJDIR := build

common_hdrs = common.h
common_objs = $(OBJDIR)/common.o

main = $(OBJDIR)/main.o
allobjs= $(common_objs) $(main)

all: main

main: $(common_hdrs) $(common_objs) $(main) $(OBJDIR)
	$(CXX) -o main $(main) $(common_objs) /usr/lib/libboost_system.so.1.48.0 $(LDFLAGS)

$(OBJDIR):
	mkdir -p $@

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

allobjs: | $(OBJDIR)

clean: 
	rm -f main
