CXX := g++
CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -pthread

INCLUDES := -I/opt/homebrew/include
LIBPATHS := -L/opt/homebrew/lib

LIBS := -lboost_system -lboost_json -lssl -lcrypto

SOURCES := kraken.cpp orderbook.cpp ringbuffer.cpp main.cpp
OBJECTS := $(SOURCES:.cpp=.o)
TARGET := kraken

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LIBPATHS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)