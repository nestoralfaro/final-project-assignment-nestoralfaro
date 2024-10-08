# Compiler
CXX = g++

# Compiler flags
# CXXFLAGS = -std=c++11 `pkg-config --cflags opencv4`
CXXFLAGS = -std=c++11 -I/opt/homebrew/include/opencv4/

# Linker flags
# LDFLAGS = `pkg-config --libs opencv4`
LDFLAGS = -L/opt/homebrew/lib/ -lopencv_core -lopencv_imgproc -lopencv_videoio

# Target executable
TARGET = openspot

# Source files
SRCS = main.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Build the target executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Build the object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean the build
clean:
	rm -f $(TARGET) $(OBJS)

# Phony targets
.PHONY: all clean
