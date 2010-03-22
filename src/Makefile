####### Compiler, tools and options
CXX           = g++
DEL_FILE      = rm -f

####### Files

SOURCES       = myprogram.cpp \
		finder.cpp 

OBJECTS       = myprogram.o \
		finder.o 

TARGET        = ../bin/myprogram

####### Build rules

all: $(TARGET)

clean:
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~

$(TARGET): libdasm.o $(OBJECTS)
	$(CXX) -o $(TARGET) libdasm.o $(OBJECTS)

####### Compile

myprogram.o: myprogram.cpp finder.h
	$(CXX) -c myprogram.cpp

finder.o: finder.cpp finder.h libdasm.h
	$(CXX) -c finder.cpp