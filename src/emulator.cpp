#include "emulator.h"

void Emulator::bind(Reader* r) 
{
	reader = r;
}
unsigned int Emulator::memory_offset()
{
	return 0;
}
