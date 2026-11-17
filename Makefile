TARGET = isaac_trainer
OBJS = main.cpp

# Используем официальную обёртку компилятора OpenOrbis
CXX = oo-clang++

CFLAGS = -O3 -Wall -fPIC
CXXFLAGS = $(CFLAGS)

all: $(TARGET).prx

$(TARGET).prx: $(OBJS)
	$(CXX) $(CXXFLAGS) -shared $(OBJS) -o $(TARGET).prx

clean:
	rm -f $(TARGET).prx
