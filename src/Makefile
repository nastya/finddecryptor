####### Compiler, tools and options
CXX		= g++
CC		= gcc
DEL_FILE	= rm -f

####### Files
OBJECTS       = myprogram.o \
		data.o \
		fdostream.o \
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

fdostream.o: fdostream.cpp fdostream.h
	$(CXX) -c fdostream.cpp

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
	./$(TARGET) ../input/cmd_exec_notepad.call4_dword_xor.exe > ../output.call4_dword_xor.txt
	./$(TARGET) ../input/cmd_exec_notepad.fnstenv_mov.exe > ../output.fnstenv_mov.txt
	./$(TARGET) ../input/cmd_exec_notepad.nonalpha.exe > ../output.nonalpha.txt
	./$(TARGET) ../input/cmd_exec_notepad.countdown.exe > ../output.countdown.txt
	./$(TARGET) ../input/cmd_exec_notepad.jmp_call_additive.exe > ../output.jmp_call_additive.txt