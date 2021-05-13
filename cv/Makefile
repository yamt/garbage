# note: use gnu make >= 4 or bsd make

# brew install opencv

# brew install make
# or
# brew install bsdmake

CXXFLAGS != pkg-config --cflags opencv4
CXXFLAGS+=-std=c++17

CXXLINKFLAGS != pkg-config --libs opencv4
#LIBS+=-lopencv_core
#LIBS+=-lopencv_videoio
#LIBS+=-lopencv_highgui

OBJS=cap.o

all: test

.cpp.o:
	$(CXX) -c $(CXXFLAGS) -o $@ $<

test: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(CXXLINKFLAGS) $(LIBS)

clean:
	rm -f test
	rm -f $(OBJS)
