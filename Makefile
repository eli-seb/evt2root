CC=g++
CFLAGS=-c -g -Wall `root-config --cflags`
LDFLAGS=`root-config --glibs`
SOURCES= evt2root.cpp ADCUnpacker.cpp mQDCUnpacker.cpp main.cpp 
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=evt2root

.PHONY: all clean

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm ./*.o ./evt2root
