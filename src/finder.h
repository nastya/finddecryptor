#ifndef FINDER_H
#define FINDER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

#include "libdasm.h" 

using namespace std;

enum Register {
	EAX,EDX,ECX,EBX,EDI,ESI,EBP,ESP,AX,DX,CX,BX,DI,SI,BP,SP,AH,DH,CH,BH,AL,DL,CL,BL
};

class Finder {
public:
	Finder();
	Finder(char *);
	~Finder();
	void find();
	void read_file(char *);
	void clear();
private:
	/**
	Finds instructions that write to memory.
	@param pos Position in binary file from which to start finding (number of byte).
	@return Returns the position of write to memory or -1 if nothing found.
	*/
	int find_memory(int pos);
	int find_jump(int);
	void backwards_traversal(int);
	void get_operands(int);
	void print_commands(vector <INSTRUCTION>*, int start=0);
	void get_commands(vector <INSTRUCTION>*, vector <int>* ,vector <int>*);
	void check1(vector <INSTRUCTION>*);
	bool check2(vector <int>*, vector <int>*);
	void check_inst(INSTRUCTION);
	void init();

	bool *regs_target;
	unsigned char *data;
	int dataSize;
	static const int RegistersCount, CommandsChangingCount;
	static const char *Registers[], *CommandsChanging[];
	static const int MaxCommandSize;
	static const Mode mode;
	static const Format format;
};

#endif