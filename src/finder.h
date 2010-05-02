#ifndef FINDER_H
#define FINDER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <algorithm>
#include <set>

#include "libdasm.h" 
#include "data.h"
#include "emulator.h"
#include "PEReader.h"

using namespace std;

struct Command {
	Command(int a = 0, INSTRUCTION inst = INSTRUCTION(), string s = "");
	int addr;
	INSTRUCTION inst;
	string str;
};

/**
  @brief
    Class finding instructions to emulate.
 */
class Finder : private Data {
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
	*/
	void find_memory(int pos);
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
	int backwards_traversal(int pos);
	/**
	Gets operands of target instruction (registers used in it).
	Saves this information in regs_target.
	@param pos Position of target instruction in binary file.
	*/
	void get_operands(int pos);
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
	*/
	void check(vector <INSTRUCTION>* instructions);
	/**
	Checks whether instruction defines one of the registers that need to be defined before the emulation and changes regs_target in corresponding way.
	@param inst Instruction to check.
	*/
	void check(INSTRUCTION inst);
	/**
	Initializes variables for further use.
	*/
	bool regs_closed();
	void init();
	void launch(int start, int pos=0);
	int verify(Command *cycle, int size);
	bool verify_changing_reg(Command *cycle, int size, int reg);
	bool is_write(INSTRUCTION *inst);
	bool is_indirect_write(Command *command, int *reg);
	void get_operands(string str);
	void smaller_to_greater_regs();
	
	vector <INSTRUCTION> instructions_after_getpc;
	int starting_point;
	int start_emul;
	int pos_getpc;
	PEReader reader;
	
	bool *regs_target; ///<registers to be defined (array which size is number of registers, regs_target[i]=true if register is to be defined and regs_target[i]=false vice versa)
	bool *regs_known;
	
	char* filename; ///<input file name
	unsigned char *data; ///<buffer containing binary file
	int dataSize; ///<size of buffer data
	Emulator emulator;
	set<int> start_positions;
	static const Mode mode; ///<mode of disassembling (here it is MODE_32)
	static const Format format; ///<format of commands (here it is Intel)

	string instruction_string(INSTRUCTION *inst, int pos=0);
	string instruction_string(BYTE *data, int pos=0);
	string instruction_string(int pos);

	/** Debug **/
	/**
	Prints commands from vector of instructions v.
	@param start Position from which to print commands.
	*/
	void print_commands(vector <INSTRUCTION>* v, int start=0);
	ofstream *log;
	/** /Debug **/
};

#endif