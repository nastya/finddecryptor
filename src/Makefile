####### Compiler, tools and options
CXX		= g++ -Wall -O2 -I.
CC		= gcc -Wall -O2 -I.
DEL_FILE	= rm -f
####### Files
OBJECTS       = myprogram.o \
		data.o \
		fdostream.o \
		libdasm.o \
		emulator_gdbwine.o \
		emulator_libemu.o \
		finder.o \
		PEReader.o

TARGET        = ../bin/myprogram

####### Build rules

$(TARGET): $(OBJECTS)
	$(CXX) -o $(TARGET) $(OBJECTS) emu/libemu.so -Wl,-rpath -Wl,emu

all: $(TARGET) test

clean:
	-$(DEL_FILE) $(OBJECTS) *~ ../output.*.txt ../log.txt ../dbg.txt

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

emulator_gdbwine.o: emulator_gdbwine.cpp emulator_gdbwine.h emulator.h
	$(CXX) -c emulator_gdbwine.cpp

emulator_libemu.o: emulator_libemu.cpp emulator_libemu.h emulator.h
	$(CXX) -c emulator_libemu.cpp

test: test_gdbwine

test_gdbwine: $(TARGET) 
	./$(TARGET) ../input/cmd_exec_notepad.avoid_utf8_tolower.exe GdbWine > ../output.avoid_utf8_tolower.txt
	./$(TARGET) ../input/cmd_exec_notepad.call4_dword_xor.exe GdbWine > ../output.call4_dword_xor.txt
	./$(TARGET) ../input/cmd_exec_notepad.countdown.exe GdbWine > ../output.countdown.txt
	./$(TARGET) ../input/cmd_exec_notepad.fnstenv_mov.exe GdbWine > ../output.fnstenv_mov.txt
	./$(TARGET) ../input/cmd_exec_notepad.jmp_call_additive.exe GdbWine > ../output.jmp_call_additive.txt
	./$(TARGET) ../input/cmd_exec_notepad.nonalpha.exe GdbWine > ../output.nonalpha.txt
	./$(TARGET) ../input/cmd_exec_notepad.shikata_ga_nai.exe GdbWine > ../output.shikata_ga_nai.txt

test_libemu: $(TARGET) 
	./$(TARGET) ../input/cmd_exec_notepad.avoid_utf8_tolower.exe LibEmu > ../output.avoid_utf8_tolower.txt
	./$(TARGET) ../input/cmd_exec_notepad.call4_dword_xor.exe LibEmu > ../output.call4_dword_xor.txt
	./$(TARGET) ../input/cmd_exec_notepad.countdown.exe LibEmu > ../output.countdown.txt
	./$(TARGET) ../input/cmd_exec_notepad.fnstenv_mov.exe LibEmu > ../output.fnstenv_mov.txt
	./$(TARGET) ../input/cmd_exec_notepad.jmp_call_additive.exe LibEmu > ../output.jmp_call_additive.txt
	./$(TARGET) ../input/cmd_exec_notepad.nonalpha.exe LibEmu > ../output.nonalpha.txt
	./$(TARGET) ../input/cmd_exec_notepad.shikata_ga_nai.exe LibEmu > ../output.shikata_ga_nai.txt