# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 `pkg-config --cflags opencv4`

# Linker flags
LDFLAGS = `pkg-config --libs opencv4`

# Target executable
TARGET = parking_spot_detector

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

# Clean generated image files
clean-images:
	rm -f *.jpg

# Phony targets
.PHONY: all clean clean-images




######## old #########

# # Compiler
# CXX = g++
#
# # Compiler flags
# CXXFLAGS = -std=c++11
#
# # Target executable
# TARGET = openspot
#
# # Source files
# SRCS = main.cpp
#
# # Object files
# OBJS = $(SRCS:.cpp=.o)
#
# # Default target
# all: $(TARGET)
#
# # Build the target executable
# $(TARGET): $(OBJS)
# 	$(CXX) $(CXXFLAGS) -o $@ $^
#
# # Build the object files
# %.o: %.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@
#
# # Clean the build
# clean:
# 	rm -f $(TARGET) $(OBJS)
#
# # Phony targets
# .PHONY: all clean
