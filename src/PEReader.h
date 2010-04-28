#ifndef PEREADER_H
#define PEREADER_H

struct ste
{
	char name[8];
	int virt_addr;
	int virt_size;
	int raw_offset;
};
typedef struct ste entry;
class PEReader
{
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
	void swap(entry* entry1, entry* entry2);
	int number_of_sections;
	int base;
	int entry_point;
	entry* table;
};
#endif