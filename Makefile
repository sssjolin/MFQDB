CXX=g++
CXXFLAGS+=-g -std=c++11
LDFLAGS+=
OBJS = Source1.o
LIBS=-lpq
cs562: 	$(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LDFLAGS) $(LIBS)
clean: 
	rm -f cs562 $(OBJS)
