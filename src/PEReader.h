#ifndef PEREADER_H
#define PEREADER_H

#include <string>

using namespace std;

class PEReader
{
	struct entry
	{
		char name[8];
		int virt_addr;
		int virt_size;
		int raw_offset;
	};
public:
	PEReader();
	~PEReader();
	void init(string name);
	unsigned char *pointer();
	string name();
	int size();
	int start();
	int entrance();
	int map(int addr);
	bool is_within_one_block(int a, int b);
	void print_table();
private:
	void read();
	void parse();
	void sort();
	int get(int pos, int size=4);

	int number_of_sections;
	int base;
	int entry_point;
	entry* table;
	string filename; ///<input file name
	unsigned char *data; ///<buffer containing binary file
	int dataSize; ///<size of buffer data
};
#endif