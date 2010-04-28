#include "data.h"

const int Data::MaxCommandSize = 10;
const char* Data::Registers [] = {"eax","edx","ecx","ebx","edi","esi","ebp","esp","eip","ax","dx","cx","bx","di","si","bp","sp",
"ah","dh","ch","bh","al","dl","cl","bl" }; 
const char* Data::CommandsChanging [] = {"xor","add","and","or","sub","mul","imul","div","mov","pop"};
const int Data::RegistersCount = sizeof(Registers)/sizeof(Registers[0]);
const int Data::CommandsChangingCount = sizeof(CommandsChanging)/sizeof(CommandsChanging[0]); 
