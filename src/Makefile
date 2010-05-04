####### Compiler, tools and options
CXX		= g++ -Wall -O2 -I.
CC		= gcc -Wall -O2 -I.
DEL_FILE	= rm -f
####### Files
OBJECTS		= myprogram.o \
		  finder.o \
		  data.o \
		  PEReader.o \
		  fdostream.o \
		  libdasm.o \
		  emulator_gdbwine.o \
		  emulator_libemu.o

LIBS		= ../lib/libdasm.so \
		  ../lib/emulator_gdbwine.so \
		  ../lib/emulator_libemu.so

TARGET		= ../bin/myprogram
INPUT		= ../input/cmd_exec_notepad
OUTPUT		= ../log/output

####### Build rules

$(TARGET): $(OBJECTS) $(LIBS)
	mkdir -p ../bin
	$(CXX) -o $(TARGET) myprogram.o data.o finder.o PEReader.o ../lib/libdasm.so ../lib/emulator_gdbwine.so ../lib/emulator_libemu.so -Wl,-rpath -Wl,$(CURDIR)/../lib

all: $(TARGET) doc test

clean:
	-$(DEL_FILE) $(OBJECTS) *~

clear: clean
	-$(DEL_FILE) -r ../bin ../lib ../log ../doc

doc:
	-$(DEL_FILE) ../doc/
	mkdir -p ../doc/
	doxygen Doxyfile

####### Compile

myprogram.o: myprogram.cpp finder.h
	$(CXX) -c myprogram.cpp

finder.o: finder.cpp finder.h libdasm.h emulator.h PEReader.h
	$(CXX) -c finder.cpp -I ../libemu

data.o: data.cpp data.h
	$(CXX) -c data.cpp

PEReader.o: PEReader.cpp PEReader.h
	$(CXX) -c PEReader.cpp

fdostream.o: fdostream.cpp fdostream.h
	$(CXX) -c fdostream.cpp

libdasm.o: libdasm.c
	$(CC) -c libdasm.c

emulator_gdbwine.o: emulator_gdbwine.cpp emulator_gdbwine.h emulator.h
	$(CXX) -c emulator_gdbwine.cpp

emulator_libemu.o: emulator_libemu.cpp emulator_libemu.h emulator.h
	$(CXX) -c emulator_libemu.cpp -I ../libemu

../lib/libdasm.so: libdasm.o
	mkdir -p ../lib
	$(CC) -shared -o $@ libdasm.o

../lib/emulator_gdbwine.so: emulator_gdbwine.o fdostream.o
	mkdir -p ../lib
	$(CXX) -shared -o $@ emulator_gdbwine.o fdostream.o

../lib/emulator_libemu.so: emulator_libemu.o ../libemu/libemu.so
	mkdir -p ../lib
	$(CXX) -shared -o $@ emulator_libemu.o ../libemu/libemu.so -Wl,-rpath -Wl,$(CURDIR)/../libemu

test: test_gdbwine

test_gdbwine: $(TARGET)
	mkdir -p ../log
	./$(TARGET) $(INPUT).avoid_utf8_tolower.exe GdbWine > $(OUTPUT).avoid_utf8_tolower.gdbwine.txt
	./$(TARGET) $(INPUT).call4_dword_xor.exe GdbWine > $(OUTPUT).call4_dword_xor.gdbwine.txt
	./$(TARGET) $(INPUT).countdown.exe GdbWine > $(OUTPUT).countdown.gdbwine.txt
	./$(TARGET) $(INPUT).fnstenv_mov.exe GdbWine > $(OUTPUT).fnstenv_mov.gdbwine.txt
	./$(TARGET) $(INPUT).jmp_call_additive.exe GdbWine > $(OUTPUT).jmp_call_additive.gdbwine.txt
	./$(TARGET) $(INPUT).nonalpha.exe GdbWine > $(OUTPUT).nonalpha.gdbwine.txt
	./$(TARGET) $(INPUT).shikata_ga_nai.exe GdbWine > $(OUTPUT).shikata_ga_nai.gdbwine.txt

test_libemu: $(TARGET)
	mkdir -p ../log
	./$(TARGET) $(INPUT).avoid_utf8_tolower.exe LibEmu > $(OUTPUT).avoid_utf8_tolower.libemu.txt
	./$(TARGET) $(INPUT).call4_dword_xor.exe LibEmu > $(OUTPUT).call4_dword_xor.libemu.txt
	./$(TARGET) $(INPUT).countdown.exe LibEmu > $(OUTPUT).countdown.libemu.txt
	./$(TARGET) $(INPUT).fnstenv_mov.exe LibEmu > $(OUTPUT).fnstenv_mov.libemu.txt
	./$(TARGET) $(INPUT).jmp_call_additive.exe LibEmu > $(OUTPUT).jmp_call_additive.libemu.txt
	./$(TARGET) $(INPUT).nonalpha.exe LibEmu > $(OUTPUT).nonalpha.libemu.txt
	./$(TARGET) $(INPUT).shikata_ga_nai.exe LibEmu > $(OUTPUT).shikata_ga_nai.libemu.txt