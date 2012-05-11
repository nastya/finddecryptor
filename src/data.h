#ifndef DATA_H
#define DATA_H

/**
@brief
Some necessary data (about registers and commands).
*/

class Data {
public:
	///Enum type - observed registers
	enum Register {
		EIP,EBP,ESP,ESI,EDI,
		IP,BP,SP,SI,DI,
		EAX,EBX,ECX,EDX,
		AX,BX,CX,DX,
		AH,BH,CH,DH,
		AL,BL,CL,DL,
		ES, DS, FS, GS, CS, SS,
		NOREG,
		HASFPU
	};

	static const unsigned int RegistersCount; ///<an amount of registers (only observed registers)
	static const char *Registers[]; ///<observed registers
	static const unsigned int MaxCommandSize; ///<maximum size of command in 32-bit architecture
};

#endif 
