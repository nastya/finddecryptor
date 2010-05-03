#include "PEReader.h"

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstring>

using namespace std;

PEReader::PEReader()
{
	filename = "";
	table = NULL;
	data = NULL;
	dataSize = 0;
}
PEReader::~PEReader()
{
	delete [] table;
	delete [] data;
}
void PEReader::init(string name)
{
	filename = name;
	read();
	parse();
}
void PEReader::read()
{
	delete [] data;
	data = NULL;
	dataSize = 0;
	ifstream s(filename.c_str());
	if (!s.good() || s.eof() || !s.is_open()) 
	{
		cerr << "Error opening file." << endl;
		exit(0);
	}
	s.seekg(0, ios_base::beg);
	ifstream::pos_type begin_pos = s.tellg();
	s.seekg(0, ios_base::end);
	dataSize = static_cast<int>(s.tellg() - begin_pos);
	s.seekg(0, ios_base::beg);
	data = new unsigned char[dataSize];
	s.read((char *) data,dataSize);
	s.close();
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
unsigned char* PEReader::pointer() {
	return data;
}
string PEReader::name() {
	return filename;
}
int PEReader::size() {
	return dataSize;
}
int PEReader::start()
{
	return table[0].raw_offset;
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
void PEReader::print_table()
{
	for (int i=0;i<number_of_sections;i++)
	{
		cerr<<table[i].name<<" ";
		cerr<<hex<<table[i].virt_addr<<" ";
		cerr<<hex<<table[i].raw_offset<<endl;
	}
}
