#ifndef FINDER_H
#define FINDER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cstring>
#include <algorithm>
#include <set>

#include "libdasm.h" 
#include "data.h"
#include "timer.h"
#include "emulator.h"
#include "reader_pe.h"

#define FINDER_LOG /// Write to logfile.
//#define FINDER_ONCE /// Stop after first found decryption routine.

using namespace std;

/**
  @brief
    Class finding instructions to emulate.
 */
class Finder : private Data {
public:
	/**
	  Struct to keep information about instruction (its address and instruction itself)
	*/
	struct Command {
		Command(int a = 0, INSTRUCTION inst = INSTRUCTION());
		int addr;
		INSTRUCTION inst;
	};

	/**
	@param type Type of the emulator. Possible values: 0(GdbWine), 1(LibEmu).
	*/
	Finder(int type=0);
	/**
	Destructor of class Finder.
	*/
	~Finder();
	/**
	Loads a file.
	@param name Name of input file.
	@param guessType Try to guess binary type.
	*/
	void load(string name, bool guessType=false);
	/**
	Links to data.
	@param data Pointer to memory area.
	@param dataSize Size of memory area.
	@param guessType Try to guess binary type.
	*/
	void link(const unsigned char *data, uint dataSize, bool guessType=false);
	/**
	Applies a reader. Common part of load() and link() functions.
	@param reader Reader to apply.
	@param guessType Try to guess binary type.
	*/
	void apply_reader(Reader *reader, bool tryTypes=false);
	/**
	Wrap on functions finding writes to memory and indirect jumps.
	*/
	int find();
	/**
	Cleans memory before exiting the program. Is called by destructor.
	*/
	void clear();
private:
	/**
	Finds instructions writing to memory and indirect jumps (via disassembling sequence of bytes starting from pos).
	@param pos Position in binary file from which to start finding (number of byte).
	*/
	void find_memory_and_jump(int pos);
	/**
	Implements techniques of backwards traversal.
	Disassembles bytes in reverse order from pos. Founds the most appropriate chain using special rules (all the variables of target instruction should be defined within that chain) and prints it. 
	@param pos Starting point of the process.
	*/
	int backwards_traversal(int pos);
	/**
	Gets operands of given instruction (registers used in it).
	Saves this information in regs_target.
	@param inst Given instruction
	*/
	void get_operands(INSTRUCTION *inst);
	/**
	Checks every instruction in vector instructions. Changes regs_target and regs_known respectively.
	@param instructions Vector of instructions to be checked.
	*/
	void check(vector <INSTRUCTION>* instructions);
	/**
	Checks whether instruction defines one of the registers that need to be defined before the emulation and changes regs_target regs_known in corresponding way.
	@param inst Instruction to check.
	*/
	void check(INSTRUCTION *inst);
	/**
	  Adds new dependencies in regs_target (if any registers make influence on given operand).
	  @param op Operand of some instruction.
	*/
	void add_target(OPERAND *op);
	/**
	  Translates registers from libdasm format to the neccessary format used here. 
	  @param code register in libdasm format (it means its number as it is a presented in enum format)
	*/
	int int_to_reg(int code);
	/**
	 @return Returns true if all dependencies are found and false vice versa.
	*/
	bool regs_closed();
	/**
	 Function which works with emulator. Makes emulator emulate found chain of instruction and looks for the loop. If neccessary restarts the process of finding dependencies and restarts emulator.
	 @return pos Position in input file from which emulation is started.
	*/
	void launch(int pos=0);
	/**
	  Checks the cycle found for the presence of decription routine.
	  @param cycle Cycle found (represents sequence of entities named Command).
	  @param size A number of lines in cycle.
	  @return Returns the number of line where indirect write happens.
	*/
	int verify(Command *cycle, int size);
	/**
	  Checks the cycle found for the presence of instructions changing register in target instruction.
	  @param inst The target instruction to check.
	  @param cycle Cycle found (represents sequence of entities named Command).
	  @param size A number of lines in cycle.
	  @return true if such instruction is found and false vice versa.
	  */
	bool verify_changing_reg(INSTRUCTION *inst, Command *cycle, int size);
	/**
	@param inst Given instruction.
	@return Returns true if given instruction rewrites at least one of its operands. 
	*/
	bool is_write(INSTRUCTION *inst);
	/**
	@param inst Given instruction.
	@return Returns true if given instruction rewrites at least one of its operands and used write is indirect.
	@sa is_write
	*/
	bool is_write_indirect(INSTRUCTION *inst);
	/**
	@param inst Given instruction.
	@param reg Pointer to register which is used in indirect addressing.
	@sa is_write_indirect
	@return Return value is the same as in is_write_indirect function. Additionally saves information about register in second parameter.
	*/
	bool get_write_indirect(INSTRUCTION *inst, int *reg);
	
	vector <INSTRUCTION> instructions_after_getpc;///<instructions between seeding and target instruction
	int pos_getpc; ///<position of seeding instruction in the inputfile
	
	bool *regs_target; ///<registers to be defined (array which size is number of registers, regs_target[i]=true if register is to be defined and regs_target[i]=false vice versa)
	bool *regs_known; ///registers which are already defined (array which size is number of registers, regs_known[i]=true if register was defined and regs_target[i]=false vice versa)
	
	Reader *reader; ///<saves neccessary information about structure of input from its header 
	Emulator *emulator; ///<emulator used
	set<int> start_positions;///<positions where target instructions are alredy found
	static const Mode mode; ///<mode of disassembling (here it is MODE_32)
	static const Format format; ///<format of commands (here it is Intel)
	static const int maxBackward; ///<limit for backwards traversal
	int am_back; ///<amount of commands found by backwards traversal
	int matches; ///<number of matches found
	Command cycle[256]; // TODO: fix. It should be a member of the Finder::launch(). Here because of qemu lags.

	/**
	  @param pos Position in input file from which we get instruction.
	  @param inst Pointer instruction the function gets.
	  @return Length of instruction.
	*/
	int instruction(INSTRUCTION *inst, int pos=0); 
	/**
	  @param pos Position in input file from which we get instruction.
	  @param inst Pointer to instruction the function gets.
	  @return String containing the instruction.
	*/
	string instruction_string(INSTRUCTION *inst, int pos=0);
	/**
	  @param pos Position in input file from which we get instruction.
	  @return String containing the instruction.
	*/
	string instruction_string(int pos);

	/** Debug **/
	/**
	Prints commands from vector of instructions v.
	@param start Position from which to print commands.
	*/
	void print_commands(vector <INSTRUCTION>* v, int start=0);
	ofstream *log;///<stream used to write all service information.
	/** /Debug **/
};

#endif