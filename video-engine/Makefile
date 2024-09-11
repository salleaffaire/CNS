CXXFLAGS = -g -std=c++17 -I ../NDI_SDK/include -I/usr/include/opencv4
LDLIBS = -L ../NDI_SDK/lib/x86_64-linux-gnu -lndi -pthread

PRGM  = ve
SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)
DEPS := $(OBJS:.o=.d)

.PHONY: all clean

all: $(PRGM)

$(PRGM): $(OBJS)
	$(CXX) $(OBJS) $(LDLIBS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf $(OBJS) $(DEPS) $(PRGM)

-include $(DEPS)