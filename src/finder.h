#ifndef FINDER_H
#define FINDER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

#include "libdasm.h" 

using namespace std;

///Enum type - observed registers
enum Register {
	EAX,EDX,ECX,EBX,EDI,ESI,EBP,ESP,AX,DX,CX,BX,DI,SI,BP,SP,AH,DH,CH,BH,AL,DL,CL,BL
};

/**
  @brief
    Class finding instructions to emulate.
 */
class Finder {
public:
	/**
	Constructor of class Finder. Calls initialization function.
	*/
	Finder();
	/**
	Constructor of class Finder. Calls initialization and reading fucntions.
	@param name Name of input file.
	*/
	Finder(char * name);
	/**
	Destructor of class Finder.
	*/
	~Finder();
	/**
	Wrap on functions finding writes to memory and indirect jumps.
	*/
	void find();
	/**
	Reads input binary file into buffer.
	@param name Name of input file.
	*/
	void read_file(char *name);
	/**
	Cleans memory before exiting the program. Is called by destructor.
	*/
	void clear();
private:
	/**
	Finds instructions that write to memory (via disassembling sequence of bytes starting from pos).
	@param pos Position in binary file from which to start finding (number of byte).
	@return Returns the position of write to memory or -1 if nothing found.
	*/
	int find_memory(int pos);
	/**
	Finds indirect jumps (via disassembling sequence of bytes starting from pos).
	@param pos Position in binary file from which to start finding (number of byte).
	@return Returns the position of indirect or -1 if nothing found.
	*/
	int find_jump(int pos);
	/**
	Implements techniques of backwards traversal.
	Disassembles bytes in reverse order from pos. Founds the most appropriate chain using special rules (all the variables of target instruction should be defined within that chain) and prints it. 
	@param pos Starting point of the process.
	*/
	void backwards_traversal(int pos);
	/**
	Gets operands of target instruction (registers used in it).
	Saves this information in regs_target.
	@param pos Position of target instruction in binary file.
	*/
	void get_operands(int pos);
	/**
	Prints commands from vector of instructions v.
	@param start Position from which to print commands.
	*/
	void print_commands(vector <INSTRUCTION>* v, int start=0);
	/**
	Forms a chain of commands from the information containing in num_commands and prev vectors (they are formed in backwards_traversal).
	@param num_commands - vector containing the starting positions of instructions (reference to first byte of instruction).
	@param prev - vector containing the positions of previous instructions corresponding to num_commands (reference to first byte of instruction).
	@returns commands - Vector containing chain of commands. 
	*/
	void get_commands(vector <INSTRUCTION>* commands, vector <int>* num_commands, vector <int>* prev);
	/**
	Checks every instruction in vector instructions.
	@param instruction Vector of instructions to be checked.
	@sa check_inst
	*/
	void check1(vector <INSTRUCTION>* instructions);
	/**
	Check before ending backwards traversal.
	@param queue - vector containing the starting positions of instructions (reference to first byte of instruction).
	@param prev - vector containing the positions of previous instructions corresponding to num_commands (reference to first byte of instruction).
	@return Returs true if all of the operands in target instruction are defined within the found chain and false if not.
	*/
	bool check2(vector <int>* queue, vector <int>* prev);
	/**
	Checks whether instruction defines one of the registers that need to be defined before the emulation and changes regs_target in corresponding way.
	@param inst Instruction to check.
	*/
	void check_inst(INSTRUCTION inst);
	/**
	Initializes variables for further use.
	*/
	void init();

	bool *regs_target; ///<registers to be defined (array which size is number of registers, regs_target[i]=true if register is to be defined and regs_target[i]=false vice versa)
	unsigned char *data; ///<buffer containing binary file
	int dataSize; ///<size of buffer data
	static const int RegistersCount, ///<an amount of registers (only observed registers)
			 CommandsChangingCount; ///<an amount of commands writing to memory (only observed commands, not all)
	static const char *Registers[], ///<observed registers
			  *CommandsChanging[]; ///<observed commands writing to memory
	static const int MaxCommandSize; ///<maximum size of command in 32-bit architecture
	static const Mode mode; ///<mode of disassembling (here it is MODE_32)
	static const Format format; ///<format of commands (here it is Intel)
};

#endif