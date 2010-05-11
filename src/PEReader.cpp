#include "PEReader.h"

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstring>

using namespace std;

PEReader::PEReader() : Reader()
{
	table = NULL;
}
PEReader::PEReader(const Reader *reader) : Reader(reader)
{
	table = NULL;
	/// We know that we have a loaded file here.
	parse();
}
PEReader::~PEReader()
{
	delete [] table;
}
bool PEReader::is_of_type(const Reader *reader)
{
	/// TODO: write some logic here
	return false;
}
void PEReader::load(string name)
{
	Reader::load(name);
	parse();
}
void PEReader::parse()
{	
	entry_point = get(data[60]+40);
	base = get(data[60]+52);
	number_of_sections = get(data[60]+6,2);
	int size = get(data[60]+20,2);
	delete [] table;
	table = new entry [number_of_sections];
	for(int i=data[60]+24+size,k=0;k<number_of_sections;k++,i+=40)
	{
		memcpy(table[k].name,&(data[i]),8);
		table[k].virt_size = get(i+8);
		table[k].virt_addr = get(i+12);
		table[k].raw_offset = get(i+20);
	}
	sort();
	dataStart = table[0].raw_offset;
}
void PEReader::sort()
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
int PEReader::get(int pos, int size) {
	int x = data[pos+size-1];
	for (int i=size-2; i>=0; i--)
		x = x*16*16 + data[pos+i];
	return x;
}
void PEReader::print_table()
{
	cerr << "Base: 0x" << hex << base << endl;
	for (int i=0;i<number_of_sections;i++)
		cerr << table[i].name << " 0x" << hex << table[i].virt_addr << " 0x" << hex << table[i].raw_offset << endl;
}
int PEReader::entrance()
{
	return entry_point + base;
}
int PEReader::map(int addr)
{
	int k=0;
	while ((k < number_of_sections) && (addr >= table[k].raw_offset))
		k++;
	k--;		
	return addr-table[k].raw_offset+table[k].virt_addr+base;
}
bool PEReader::is_within_one_block(int a,int b)
{
	for (int i=0;i<number_of_sections;i++)
		if ((a>=table[i].virt_addr+base)&&(a<table[i].virt_addr+base+table[i].virt_size)&&
			(b>=table[i].virt_addr+base)&&(b<table[i].virt_addr+base+table[i].virt_size))
			return true;
	return false;
}
