# cross-compilation (opencv4 cross-compiled from source found in host)
# source: https://learnopencv.com/build-and-install-opencv-4-for-raspberry-pi/
CC=$(TARGET_CROSS)g++
ifeq($(STAGING_DIR),)
	# native compilation
	CC=g++
	CFLAGS=-std=c++11 `pkg-config --cflags opencv4`
	LDFLAGS=`pkg-config --libs opencv4`
else
	CFLAGS=-I$(STAGING_DIR)/include/opencv4/
	$(info CFLAGS=$(CFLAGS))
	# -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_videoio
	LDFLAGS=-L$(STAGING_DIR)/lib/ $(LOPTS)
	$(info LDFLAGS=$(LDFLAGS))
endif

TARGET=openspot
SRCS=main.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) -o $@ $(SRCS) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean install
