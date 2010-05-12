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
}
Reader::Reader(const Reader *reader)
{
	filename = reader->filename;
	dataSize = reader->dataSize;
	dataStart = reader->dataStart;
	data = reader->data;
}
Reader::~Reader()
{
	delete[] data;
}
string Reader::name() {
	return filename;
}
void Reader::load(string name)
{
	filename = name;
	read();
}
void Reader::read()
{
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
	dataSize = static_cast<int>(s.tellg() - begin_pos);
	s.seekg(0, ios_base::beg);
	data = new unsigned char[dataSize];
	s.read((char *) data,dataSize);
	s.close();
}
unsigned char* Reader::pointer(bool nohead) const {
	return data + (nohead ? dataStart : 0);
}
int Reader::size(bool nohead) const {
	return dataSize - (nohead ? dataStart : 0);
}
int Reader::start()
{
	return dataStart;
}
int Reader::entrance()
{
	return 0;
}
int Reader::map(int addr)
{
	return addr;
}
bool Reader::is_within_one_block(int a, int b)
{
	return (a>=dataStart) && (b>=dataStart) && (a<dataSize) && (b<dataSize);
}