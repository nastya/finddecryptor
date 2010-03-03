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
	int my_pos(char*, const char*);
	void read_file(char *);
	void clear_data();
	vector<string> * get_operands(int);
	char* to_char(string);
private:
	void init();
	Format format;
	unsigned char * data;
	int dataSize;
	int RegistersCount, ComandsChangingCount;
	static const char* ComandsChanging[];
	static const char* Registers[];
};

#endif