#include "data.h"

const int Data::MaxCommandSize = 10;
const char* Data::Registers [] = {
	"eip","ebp","esp","esi","edi",
	"ip","bp","sp","si","di",
	"eax","ebx","ecx","edx",
	"ax","bx","cx","dx",
	"ah","bh","ch","dh",
	"al","bl","cl","dl"
}; 
const int Data::RegistersCount = sizeof(Registers)/sizeof(Registers[0]);
