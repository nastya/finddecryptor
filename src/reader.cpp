#include "reader.h"

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstring>

using namespace std;

Reader::Reader()
{
	filename = "";
	dataSize = 0;
	dataStart = 0;
	data = NULL;
	indirect = false;
}
Reader::Reader(const Reader *reader)
{
	filename = reader->filename;
	dataSize = reader->dataSize;
	dataStart = reader->dataStart;
	data = reader->data;
	indirect = reader->indirect;
}
Reader::~Reader()
{
	if (indirect)
		delete[] data;
}
string Reader::name() {
	return filename;
}
void Reader::load(string name)
{
	filename = name;
	read();
	parse();
}
void Reader::link(unsigned char *data, uint dataSize)
{
	if (indirect)
		delete[] data;
	filename = "direct memory access";
	indirect = false;
	this->data = data;
	this->dataSize = dataSize;
	parse();
}
void Reader::read()
{
	if (indirect)
		delete[] data;
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
	dataSize = static_cast<uint>(s.tellg() - begin_pos);
	s.seekg(0, ios_base::beg);
	indirect = true;
	data = new unsigned char[dataSize];
	s.read((char *) data,dataSize);
	s.close();
}
void Reader::parse()
{}
unsigned char* Reader::pointer(bool nohead) const {
	return data + (nohead ? dataStart : 0);
}
uint Reader::size(bool nohead) const {
	return dataSize - (nohead ? dataStart : 0);
}
uint Reader::start()
{
	return dataStart;
}
uint Reader::entrance()
{
	return 0;
}
uint Reader::map(uint addr)
{
	return addr;
}
bool Reader::is_valid(uint addr) {
	return (0<=addr) && (addr<dataSize);
}
bool Reader::is_within_one_block(uint a, uint b)
{
	return (a>=dataStart) && (b>=dataStart) && (a<dataSize) && (b<dataSize);
}