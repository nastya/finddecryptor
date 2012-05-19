####### Compiler, tools and options
CXX		= g++ -Wall -fPIC -O2 -I.
CC		= gcc -Wall -fPIC -O2 -I.
DEL_FILE	= rm -f
#FINDER_FLAGS	= -DFINDER_LOG -DTRY_READERS -DBACKEND_LIBEMU -DBACKEND_QEMU -DBACKEND_GDBWINE
#EMULATOR_FILES	= ../lib/libemulator_libemu.so ../lib/libemulator_qemu.so ../lib/libemulator_gdbwine.so
#EMULATORS	= -lemulator_libemu -lemulator_qemu -lemulator_gdbwine
FINDER_FLAGS	= -DTRY_READERS -DBACKEND_LIBEMU
EMULATOR_FILES	= ../lib/libemulator_libemu.so
EMULATORS	= -lemulator_libemu
####### Files
OBJECTS		= main.o \
		  finder.o \
		  finder-cycle.o \
		  finder-getpc.o \
		  finder-libemu.o \
		  finddecryptor.o \
		  data.o \
		  reader.o \
		  reader_pe.o \
		  fdostream.o \
		  timer.o \
		  emulator.o \
		  emulator_qemu.o \
		  emulator_gdbwine.o \
		  emulator_libemu.o

TARGET		= ../bin/finddecryptor
INPUT		= ../input/
OUTPUT		= ../log/output

####### Build rules

bin: $(TARGET)

lib: $(TARGET_LIB)

all: $(TARGET) $(TARGET_LIB) doc test

clean:
	-$(DEL_FILE) $(OBJECTS) *~

clear: clean
	-$(DEL_FILE) -r ../bin ../lib ../log ../doc

doc:
	-$(DEL_FILE) ../doc/
	mkdir -p ../doc/
	doxygen Doxyfile

####### Compile

main.o: main.cpp finder-cycle.h
	$(CXX) -c main.cpp

finder.o: finder.cpp finder.h emulator.h reader_pe.h timer.h Makefile
	$(CXX) -c finder.cpp $(FINDER_FLAGS)

finder-cycle.o: finder-cycle.cpp finder-cycle.h finder.h Makefile
	$(CXX) -c finder-cycle.cpp $(FINDER_FLAGS)

finder-getpc.o: finder-getpc.cpp finder-getpc.h finder.h Makefile
	$(CXX) -c finder-getpc.cpp $(FINDER_FLAGS)

finder-libemu.o: finder-libemu.cpp finder-libemu.h finder.h Makefile
	$(CXX) -c finder-libemu.cpp $(FINDER_FLAGS)

finddecryptor.o: finddecryptor.cpp finddecryptor.h finder-cycle.h
	$(CXX) -c finddecryptor.cpp

data.o: data.cpp data.h
	$(CXX) -c data.cpp

reader.o: reader.cpp reader.h
	$(CXX) -c reader.cpp

reader_pe.o: reader_pe.cpp reader_pe.h
	$(CXX) -c reader_pe.cpp

fdostream.o: fdostream.cpp fdostream.h
	$(CXX) -c fdostream.cpp

timer.o: timer.cpp
	$(CXX) -c timer.cpp

emulator.o: emulator.cpp
	$(CXX) -c emulator.cpp

emulator_gdbwine.o: emulator_gdbwine.cpp emulator_gdbwine.h emulator.h
	$(CXX) -c emulator_gdbwine.cpp

emulator_libemu.o: emulator_libemu.cpp emulator_libemu.h emulator.h
	$(CXX) -c emulator_libemu.cpp

emulator_qemu.o: emulator_qemu.cpp emulator_qemu.h emulator.h
	$(CXX) -c emulator_qemu.cpp

../lib/libemulator_gdbwine.so: emulator_gdbwine.o emulator.o fdostream.o
	mkdir -p ../lib
	$(CXX) -shared -o $@ emulator_gdbwine.o emulator.o fdostream.o

../lib/libemulator_libemu.so: emulator_libemu.o emulator.o
	mkdir -p ../lib
	$(CXX) -shared -o $@ emulator_libemu.o emulator.o -lemu

../lib/libemulator_qemu.so: emulator_qemu.o emulator.o ../qemu/libqemu-stepper.so
	mkdir -p ../lib
	$(CXX) -shared -o $@ emulator_qemu.o emulator.o -lqemu-stepper -L$(CURDIR)/../qemu -Wl,-rpath -Wl,$(CURDIR)/../qemu

../lib/libfinddecryptor.so: data.o finder.o finder-cycle.o finder-getpc.o finder-libemu.o reader.o reader_pe.o timer.o finddecryptor.o $(EMULATOR_FILES)
	mkdir -p ../lib
	$(CXX) -shared -o $@ data.o finder.o finder-cycle.o finder-getpc.o finder-libemu.o reader.o reader_pe.o timer.o finddecryptor.o -ldasm $(EMULATORS) -L$(CURDIR)/../lib -Wl,-rpath -Wl,$(CURDIR)/../lib

$(TARGET): main.o ../lib/libfinddecryptor.so
	mkdir -p ../bin ../log
	$(CXX) -o $@ main.o -lfinddecryptor -L$(CURDIR)/../lib -Wl,-rpath -Wl,$(CURDIR)/../lib

test: test_libemu

test_gdbwine: $(TARGET)
	mkdir -p ../log
	./$(TARGET) $(INPUT)cmd_exec_notepad.avoid_utf8_tolower.exe GdbWine > $(OUTPUT).avoid_utf8_tolower.gdbwine.txt
	./$(TARGET) $(INPUT)cmd_exec_notepad.call4_dword_xor.exe GdbWine > $(OUTPUT).call4_dword_xor.gdbwine.txt
	./$(TARGET) $(INPUT)cmd_exec_notepad.countdown.exe GdbWine > $(OUTPUT).countdown.gdbwine.txt
	./$(TARGET) $(INPUT)cmd_exec_notepad.fnstenv_mov.exe GdbWine > $(OUTPUT).fnstenv_mov.gdbwine.txt
	./$(TARGET) $(INPUT)cmd_exec_notepad.jmp_call_additive.exe GdbWine > $(OUTPUT).jmp_call_additive.gdbwine.txt
	./$(TARGET) $(INPUT)cmd_exec_notepad.nonalpha.exe GdbWine > $(OUTPUT).nonalpha.gdbwine.txt
	./$(TARGET) $(INPUT)cmd_exec_notepad.shikata_ga_nai.exe GdbWine > $(OUTPUT).shikata_ga_nai.gdbwine.txt
	./$(TARGET) $(INPUT)W32Nea_fast_encr.exe GdbWine > $(OUTPUT).W32Nea_fast_encr.gdbwine.txt
	./$(TARGET) $(INPUT)blob.seven_routines.blob GdbWine > $(OUTPUT).blob.seven_routines.gdbwine.txt

test_libemu: $(TARGET)
	mkdir -p ../log
	./$(TARGET) $(INPUT)cmd_exec_notepad.avoid_utf8_tolower.exe LibEmu > $(OUTPUT).avoid_utf8_tolower.libemu.txt
	./$(TARGET) $(INPUT)cmd_exec_notepad.call4_dword_xor.exe LibEmu > $(OUTPUT).call4_dword_xor.libemu.txt
	./$(TARGET) $(INPUT)cmd_exec_notepad.countdown.exe LibEmu > $(OUTPUT).countdown.libemu.txt
	./$(TARGET) $(INPUT)cmd_exec_notepad.fnstenv_mov.exe LibEmu > $(OUTPUT).fnstenv_mov.libemu.txt
	./$(TARGET) $(INPUT)cmd_exec_notepad.jmp_call_additive.exe LibEmu > $(OUTPUT).jmp_call_additive.libemu.txt
	./$(TARGET) $(INPUT)cmd_exec_notepad.nonalpha.exe LibEmu > $(OUTPUT).nonalpha.libemu.txt
	./$(TARGET) $(INPUT)cmd_exec_notepad.shikata_ga_nai.exe LibEmu > $(OUTPUT).shikata_ga_nai.libemu.txt
	./$(TARGET) $(INPUT)W32Nea_fast_encr.exe LibEmu > $(OUTPUT).W32Nea_fast_encr.libemu.txt
	./$(TARGET) $(INPUT)blob.seven_routines.blob LibEmu > $(OUTPUT).blob.seven_routines.libemu.txt
