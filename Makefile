# Compiler
CXX = $(BUILDROOT_OUTPUT_DIR)/host/bin/aarch64-buildroot-linux-gnu-g++

# Paths
PKG_CONFIG_PATH = $(BUILDROOT_OUTPUT_DIR)/host/aarch64-buildroot-linux-gnu/sysroot/usr/lib/pkgconfig
BUILDROOT_OUTPUT_DIR = /home/nalfaro/final-project-nestoralfaro/buildroot/output

# Compiler flags
CXXFLAGS = -std=c++11 -I$(BUILDROOT_OUTPUT_DIR)/host/aarch64-buildroot-linux-gnu/sysroot/usr/include/opencv4/
CXXFLAGS += `$(BUILDROOT_OUTPUT_DIR)/host/bin/pkg-config --cflags opencv4`

# Linker flags
LDFLAGS = `$(BUILDROOT_OUTPUT_DIR)/host/bin/pkg-config --libs opencv4`
LDFLAGS += -lstdc++

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











######### old #######
# Compiler
CC = $(CROSS_COMPILE)g++

# Compiler flags
CCFLAGS = -std=c++11 `pkg-config --cflags opencv4`

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
	$(CC) $(CCFLAGS) -o $@ $^ $(LDFLAGS)

# Build the object files
%.o: %.cpp
	$(CC) $(CCFLAGS) -c $< -o $@

# Clean the build
clean:
	rm -f $(TARGET) $(OBJS)

# Clean generated image files
clean-images:
	rm -f *.jpg

# Phony targets
.PHONY: all clean clean-images
