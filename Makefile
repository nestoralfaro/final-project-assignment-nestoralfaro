ifeq ($(CC),)
	CC = $(CROSS_COMPILE)g++
endif

ifeq ($(LD),)
	LD = ld
endif

ifeq ($(CCFLAGS),)
	CCFLAGS = -O0 -g -Wall -stdlib=libc++
endif

ifeq ($(LDFLAGS),)
	LDFLAGS = -lc++ -Wl,-rpath,/usr/local/lib
endif

CPPLIBS= -lopencv_core -lopencv_flann -lopencv_video

HFILES= 
CFILES= 
CPPFILES= main.cpp

SRCS= ${HFILES} ${CFILES}
CPPOBJS= ${CPPFILES:.cpp=.o}

all: main 

clean:
	-rm -f *.o *.d
	-rm -f main

distclean:
	-rm -f *.o *.d

main: main.o
	$(CC) $(LDFLAGS) $(CCFLAGS) -o $@ $@.o `pkg-config --libs opencv` $(CPPLIBS) 

depend:

.c.o:
	$(CC) $(CCFLAGS) -c $<

.cpp.o:
	$(CC) $(CCFLAGS) -c $<
