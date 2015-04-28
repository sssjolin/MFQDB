CC=g++
CFLAGS+=-g
LDFLAGS+=
OBJS = Source1.o
LIBS=-lpq
cs562: 	$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS) $(LIBS)
clean: 
	rm -f cs562 $(OBJS)
