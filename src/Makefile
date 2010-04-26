####### Compiler, tools and options
CXX		= g++
CC		= gcc
DEL_FILE	= rm -f

####### Files
OBJECTS       = myprogram.o \
		data.o \
		libdasm.o \
		emulator.o \
		finder.o \
		PEReader.o

TARGET        = ../bin/myprogram

####### Build rules

$(TARGET): $(OBJECTS)
	$(CXX) -o $(TARGET) $(OBJECTS)

all: $(TARGET) test

clean:
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~

####### Compile

libdasm.o: libdasm.c
	$(CC) -c libdasm.c

myprogram.o: myprogram.cpp finder.h
	$(CXX) -c myprogram.cpp

PEReader.o: PEReader.cpp PEReader.h
	$(CXX) -c PEReader.cpp

data.o: data.cpp data.h
	$(CXX) -c data.cpp

finder.o: finder.cpp finder.h libdasm.h emulator.h PEReader.h
	$(CXX) -c finder.cpp

emulator.o: emulator.cpp emulator.h
	$(CXX) -c emulator.cpp

test: $(TARGET) 
	./$(TARGET) ../input/adduserkate_encodedjmpcall.exe > ../output_adduserkate_encodedjmpcall.txt 
	./$(TARGET) ../input/cmd_exe_countdown.exe > ../output_cmd_exe_countdown.txt