#ifndef DATA_H
#define DATA_H

///Enum type - observed registers
enum Register {
	EIP,EBP,ESP,ESI,EDI,
	IP,BP,SP,SI,DI,
	EAX,EBX,ECX,EDX,
	AX,BX,CX,DX,
	AH,BH,CH,DH,
	AL,BL,CL,DL
};

class Data {
public:
	static const int RegistersCount, ///<an amount of registers (only observed registers)
			 CommandsChangingCount; ///<an amount of commands writing to memory (only observed commands, not all)
	static const char *Registers[], ///<observed registers
			  *CommandsChanging[]; ///<observed commands writing to memory
	static const int MaxCommandSize; ///<maximum size of command in 32-bit architecture
};

#endif 
