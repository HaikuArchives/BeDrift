CPPLAGS=-c -g -O0 -Wall -I/boot/home/config/include -Wno-multichar -Wshadow
LIBS=-lbe -lsocket -lbind -lpcap -ltranslation

OBJS:=	bedrift.o \
	bdcapture.o \
	conn.o \
	cp_view.o \
	cp_window.o \
	bd_window.o \
	bd_view.o

all:	bedrift

bedrift:	$(OBJS) Makefile
	$(CC) -o bedrift $(OBJS) $(LIBS)

clean:
	@rm -f *.o
	@rm -f bedrift
.cpp.o:	
	$(CC) $(CPPLAGS) $<

