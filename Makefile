TARGET = isaac_trainer
OBJS = main.cpp

CFLAGS = -O3 -Wall -fPIC
CXXFLAGS = $(CFLAGS)

all: $(TARGET).prx

$(TARGET).prx: $(OBJS)
	$(CXX) $(CXXFLAGS) -shared $(OBJS) -o $(TARGET).prx

clean:
	rm -f $(TARGET).prx
