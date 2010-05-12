#ifndef READER_H
#define READER_H

#include <string>

using namespace std;

/**
@brief
Class for reading input.
*/

class Reader
{
public:
	Reader();
	Reader(const Reader *reader);
	~Reader();

	/**
	  Load file.
	  @param name Name of input file
	*/
	virtual void load(string name);
	/**
	@return Name of the input file.
	*/
	string name();
	/**
	@return Pointer to a buffer helding an input file
	*/
	unsigned char *pointer(bool nohead=false) const;
	/**
	@return Size of input file.
	*/
	int size(bool nohead=false) const;
	/**
	  @return The position of the first instruction in file.
	*/
	int start();
	/**
	  @return Entry point of the program.
	*/
	virtual int entrance();
	/**
	  Translates address of instruction in input file into its address when program is loaded into memory.
	  @param addr Address of instruction in input file.
	*/
	virtual int map(int addr);
	/**
	@param a First address.
	@param b Second address.
	@return Returns true if these adresses are within one section
	*/
	virtual bool is_within_one_block(int a, int b);
protected:
	/**
	Reads input binary file into buffer.
	*/
	void read();
	
	string filename; ///<input file name
	unsigned char *data; ///<buffer containing binary file
	int dataSize; ///<size of buffer data
	int dataStart; ///<start of the actual data in buffer
};
#endif