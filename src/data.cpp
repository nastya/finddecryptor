#include "data.h"

namespace find_decryptor
{

const unsigned int Data::MaxCommandSize = 30;
const char* Data::Registers [] = {
	"eip","ebp","esp","esi","edi",
	"ip","bp","sp","si","di",
	"eax","ebx","ecx","edx",
	"ax","bx","cx","dx",
	"ah","bh","ch","dh",
	"al","bl","cl","dl",
	"es", "ds", "fs", "gs", "cs", "ss",
	"noreg",
	"hasfpu" // Not a register, but has to be here.
}; 
const unsigned int Data::RegistersCount = sizeof(Registers)/sizeof(Registers[0]);

} //namespace find_decryptor
