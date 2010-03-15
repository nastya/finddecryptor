#ifndef FINDER_H
#define FINDER_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <vector>
#include <cstring>

#include "libdasm.h" 

using namespace std;

class Finder {
public:
	Finder();
	Finder(char *);
	~Finder();
	void find();
	int find_memory(int);
	int find_jump(int);
	int my_pos(char*, const char*,int=0);
	void read_file(char *);
	void clear_data();
	vector<string> * get_operands(int);
	char* to_char(string);
	void backwards_traversal(int, vector <string>*);
private:
	void print_comands(vector <INSTRUCTION>* );
	vector <INSTRUCTION>* get_comands(vector <int>* ,vector <int>* );
	bool check2(vector <int>*, vector <int>*, vector <string>*);
	void delete_from_vector(vector <string>*, int);
	void check1(vector <INSTRUCTION>* , vector <string>* );
	void check_inst(INSTRUCTION, vector <string>*);
	void init();
	Format format;
	unsigned char * data;
	int dataSize;
	int RegistersCount, ComandsChangingCount;
	static const char* ComandsChanging[];
	static const char* Registers[];
	static const int MaxCommandSize;
};

#endif