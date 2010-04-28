#include "PEReader.h"
#include <iostream>

using namespace std;

PEReader::PEReader()
{
}

PEReader::~PEReader()
{
	delete [] table;
}

void PEReader::init(unsigned char* data)
{	
	entry_point=get(data,data[60]+40);
	base=get(data,data[60]+52);
	number_of_sections=get(data,data[60]+6,2);
	int size = get(data,data[60]+20,2);
	table=new entry [number_of_sections];
	int i,j,k;
	int number;
	for(i=data[60]+24+size,k=0;k<number_of_sections;k++,i+=40)
	{
		for(j=0;j<8;j++)
		{
			table[k].name[j]=data[i+j];
		}
		table[k].virt_size=get(data,i+8);
		table[k].virt_addr=get(data,i+12);
		table[k].raw_offset=get(data,i+20);
	}
	sort();
}
int PEReader::get(unsigned char *data, int pos, int size) {
	int x = data[pos+size-1];
	for (int i=size-2; i>=0; i--)
	{
		x*=16*16;
		x+=data[pos+i];
	}
	return x;
}
void PEReader::print_table()
{
	for(int i=0;i<number_of_sections;i++)
	{
		cerr<<table[i].name<<" ";
		cerr<<hex<<table[i].virt_addr<<" ";
		cerr<<hex<<table[i].raw_offset<<endl;
	}
}

void PEReader::sort()
{
	int i,j;
	for(i=number_of_sections-2;i>=0;i--)
		for(j=0;j<=i;j++)
			if (table[j].raw_offset>table[j+1].raw_offset)
				swap(&table[j],&table[j+1]);
}

void PEReader::swap(entry* a, entry* b)
{
	entry w;
	w=(*a);
	(*a)=(*b);
	(*b)=w;
}

bool PEReader::is_within_one_block(int a,int b)
{
	for(int i=0;i<number_of_sections;i++)
		if ((a>=table[i].virt_addr+base)&&(a<table[i].virt_addr+base+table[i].virt_size)&&
			(b>=table[i].virt_addr+base)&&(b<table[i].virt_addr+base+table[i].virt_size))
			return true;
	return false;
}

int PEReader::calculate_virt_addr(int addr)
{
	int k;
	k=0;
	while(k<number_of_sections&&addr>=table[k].raw_offset)
		k++;
	k--;		
	return addr-table[k].raw_offset+table[k].virt_addr+base;
}

int PEReader::get_starting_point()
{
	return entry_point+base;
}