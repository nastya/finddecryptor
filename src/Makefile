####### Compiler, tools and options
CXX           = g++
DEL_FILE      = rm -f

####### Files

SOURCES       = myprogram.cpp \
		emulator.cpp \
		finder.cpp 

OBJECTS       = myprogram.o \
		emulator.o \
		finder.o 

TARGET        = ../bin/myprogram

####### Build rules

all: $(TARGET) test

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

emulator.o: emulator.cpp
	$(CXX) -c emulator.cpp

test: $(TARGET) 
	./$(TARGET) ../input/input.exe > ../output.txt
	./$(TARGET) ../input/adduserkate_encodedjmpcall.exe > ../output.txt 
	./$(TARGET) ../input/cmd_exe_countdown.exe  > ../output.txt