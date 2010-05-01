#ifndef PEREADER_H
#define PEREADER_H

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
	void init(unsigned char *data);
	int get_starting_point();
	int calculate_virt_addr(int addr);
	void print_table();
	bool is_within_one_block(int a, int b);
private:
	int get(unsigned char *data, int pos, int size=4);
	void sort();
	int number_of_sections;
	int base;
	int entry_point;
	entry* table;
};
#endif