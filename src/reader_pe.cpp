#include "reader_pe.h"

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstring>

using namespace std;

Reader_PE::Reader_PE() : Reader()
{
	table = NULL;
}
Reader_PE::Reader_PE(const Reader *reader) : Reader(reader)
{
	table = NULL;
	/// We know that we have a loaded file here.
	parse();
}
Reader_PE::~Reader_PE()
{
	delete [] table;
}
bool Reader_PE::is_of_type(const Reader *reader)
{
	int m = reader->size();
	if (m < 0x40) return false;
	unsigned char *data = reader->pointer();
	int s = data[0x3c];
	/// TODO: check length here.
	return ((m > s+1) && (data[s]=='P') && (data[s+1]=='E'));
}
void Reader_PE::load(string name)
{
	Reader::load(name);
	parse();
}
void Reader_PE::parse()
{	
	int s = data[0x3c];
	entry_point = get(s+40);
	base = get(s+52);
	number_of_sections = get(s+6,2);
	int size = get(s+20,2);
	delete [] table;
	table = new entry [number_of_sections];
	for(int i=s+24+size,k=0;k<number_of_sections;k++,i+=40)
	{
		memcpy(table[k].name,&(data[i]),8);
		table[k].virt_size = get(i+8);
		table[k].virt_addr = get(i+12);
		table[k].raw_offset = get(i+20);
	}
	sort();
	dataStart = table[0].raw_offset;
}
void Reader_PE::sort()
{
	for (int i=number_of_sections-2;i>=0;i--)
		for (int j=0;j<=i;j++)
			if (table[j].raw_offset>table[j+1].raw_offset)
			{
				entry w = table[j];
				table[j] = table[j+1];
				table[j+1] = w;
			}
}
int Reader_PE::get(unsigned char *buff, int pos, int size)
{
	int x = buff[pos+size-1];
	for (int i=size-2; i>=0; i--)
		x = x*16*16 + buff[pos+i];
	return x;
}
int Reader_PE::get(int pos, int size)
{
	return get(data, pos, size);
}
void Reader_PE::print_table()
{
	cerr << "Base: 0x" << hex << base << endl;
	for (int i=0;i<number_of_sections;i++)
		cerr << table[i].name << " 0x" << hex << table[i].virt_addr << " 0x" << hex << table[i].raw_offset << endl;
}
int Reader_PE::entrance()
{
	return entry_point + base;
}
int Reader_PE::map(int addr)
{
	int k=0;
	while ((k < number_of_sections) && (addr >= table[k].raw_offset))
		k++;
	k--;		
	return addr-table[k].raw_offset+table[k].virt_addr+base;
}
bool Reader_PE::is_within_one_block(int a,int b)
{
	for (int i=0;i<number_of_sections;i++)
		if ((a>=table[i].virt_addr+base)&&(a<table[i].virt_addr+base+table[i].virt_size)&&
			(b>=table[i].virt_addr+base)&&(b<table[i].virt_addr+base+table[i].virt_size))
			return true;
	return false;
}
