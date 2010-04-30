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
const char* Data::CommandsChanging [] = {"xor","add","and","or","sub","mul","imul","div","mov","pop"};
const int Data::RegistersCount = sizeof(Registers)/sizeof(Registers[0]);
const int Data::CommandsChangingCount = sizeof(CommandsChanging)/sizeof(CommandsChanging[0]); 

