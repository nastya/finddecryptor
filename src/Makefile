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

$(TARGET): libdasm.o $(OBJECTS)
	$(CXX) -o $(TARGET) libdasm.o $(OBJECTS)

all: $(TARGET) test

clean:
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~

####### Compile

myprogram.o: myprogram.cpp finder.h
	$(CXX) -c myprogram.cpp

finder.o: finder.cpp finder.h libdasm.h emulator.h
	$(CXX) -c finder.cpp

emulator.o: emulator.cpp emulator.h
	$(CXX) -c emulator.cpp

test: $(TARGET) 
	./$(TARGET) ../input/adduserkate_encodedjmpcall.exe > ../output_adduserkate_encodedjmpcall.txt 
	./$(TARGET) ../input/cmd_exe_countdown.exe > ../output_cmd_exe_countdown.txt