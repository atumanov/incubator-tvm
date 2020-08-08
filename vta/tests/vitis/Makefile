VTA_HOME = ../..
TEST_PATH := $(VTA_HOME)/tests/vitis
DRIVER_PATH := $(VTA_HOME)/src/vitis

CXX = clang++-8
CXXFLAGS = --std=c++11 -I$(VTA_HOME)/include/vta/ -I$(VTA_HOME)/src/vitis -DUSE_VTA64 -Wall
LDFLAGS =

SRCS = $(DRIVER_PATH)/mem_manager.cc $(TEST_PATH)/main.cc

OBJS := $(SRCS:.cc=.o)

.PHONY: clean all

all: test

test: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -rf *.o test
