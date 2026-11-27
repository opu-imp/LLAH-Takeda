#CXX = g++ -Wall -O2 -g -DLINUX -Wno-sign-compare
CXX = g++ -std=c++17 -Wall -O2 -Wno-sign-compare

# Use OpenCV 5 (opencv5.pc) if pkg-config finds it, otherwise OpenCV 4 (opencv4.pc).
# To choose explicitly: make OPENCV_PC=opencv4
OPENCV_PC ?= $(shell pkg-config --exists opencv5 && echo opencv5 || echo opencv4)

CXXFLAGS = `pkg-config --cflags $(OPENCV_PC)`
LDFLAGS = `pkg-config --libs $(OPENCV_PC)`

PACKAGE = llahdoc

SRCS = llahdoc.cpp init.cpp fpath.cpp load.cpp block.cpp hash.cpp hcompress.cpp nears.cpp gencomb.cpp procimg.cpp outputimg.cpp hist.cpp retrieve.cpp affine.cpp disc.cpp proctime.cpp camserver.cpp proj4p.cpp projrecov.cpp sock_sv.cpp gj.cpp constdb.cpp save.cpp
HEADS = def_general.h extern.h init.h fpath.h load.h block.h hash.h hcompress.h nears.h gencomb.h procimg.h outputimg.h hist.h retrieve.h affine.h disc.h proctime.h camserver.h proj4p.h projrecov.h sock_sv.h gj.h constdb.h save.h

OBJS = $(SRCS:.cpp=.o)
FILES = Makefile $(HEADS) $(SRCS)

all:	$(PACKAGE)

$(PACKAGE):	$(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(PACKAGE) $(OBJS)
.c.o:
	$(CXX) $(CXXFLAGS)  -c $<

#$(OBJS):	$(SRCS)
#	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c $(SRCS)
$(OBJS): $(HEADS) Makefile

clean:	
	rm -f $(OBJS)

 
