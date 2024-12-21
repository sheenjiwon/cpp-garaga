OBJECTS=gpio.o led-matrix.o options-initialize.o framebuffer.o \
        thread.o graphics.o hardware-mapping.o \
        pixel-mapper.o multiplex-mappers.o server.o\
		keyboard.o

TARGET=librgbmatrix
HARDWARE_DESC?=regular
DEFINES+=$(USER_DEFINES)

DEFINES+=-DDEFAULT_HARDWARE='"$(HARDWARE_DESC)"'
INCDIR=../include
CFLAGS=-W -Wall -Wextra -Wno-unused-parameter -O3 -g -fPIC $(DEFINES) -march=native
CXXFLAGS=$(CFLAGS) -fno-exceptions -std=c++11

all : $(TARGET).a $(TARGET).so.1

$(TARGET).a : $(OBJECTS)
	$(AR) rcs $@ $^

$(TARGET).so.1 : $(OBJECTS)
	$(CXX) -shared -Wl,-soname,$@ -o $@ $^ -lpthread  -lrt -lm -lpthread

led-matrix.o: led-matrix.cc $(INCDIR)/led-matrix.h
thread.o : thread.cc $(INCDIR)/thread.h
framebuffer.o: framebuffer.cc framebuffer-internal.h
graphics.o: graphics.cc utf8-internal.h

%.o : %.cc compiler-flags
	$(CXX) -I$(INCDIR) $(CXXFLAGS) -c -o $@ $<

%.o : %.c compiler-flags
	$(CC)  -I$(INCDIR) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET).a $(TARGET).so.1

compiler-flags: FORCE
	@echo '$(CXX) $(CXXFLAGS)' | cmp -s - $@ || echo '$(CXX) $(CXXFLAGS)' > $@

.PHONY: FORCE
