#ifndef PEREADER_H
#define PEREADER_H

#include <string>

using namespace std;

/**
@brief
Class working with PE-header.
*/

class PEReader
{
	/**
	  Struct held in a tableof sections.
	*/
	struct entry
	{
		char name[8];///<name of section
		int virt_addr;///<virtual address of section
		int virt_size;///<virtual size of section
		int raw_offset;///<raw offset of section
	};
public:
	PEReader();
	~PEReader();
	/**
	  A wrap on read and parse functions
	  @param name Name of input file
	*/
	void init(string name);
	/**
	@return Pointer to a buffer helding an input file
	*/
	unsigned char *pointer();
	/**
	@return Name of the input file.
	*/
	string name();
	/**
	@return Size of input file.
	*/
	int size();
	/**
	  @return The position of the first instruction in file.
	*/
	int start();
	/**
	  @return Entry point of the program.
	*/
	int entrance();
	/**
	  Translates address of instruction in input file into its address when program is loaded into memory.
	  @param addr Address of instruction in input file.
	*/
	int map(int addr);
	/**
	@param a First address.
	@param b Second address.
	@return Returns true if these adresses are within one section
	*/
	bool is_within_one_block(int a, int b);
	/**
	  Prints table of sections
	*/
	void print_table();
private:
	/**
	Reads input binary file into buffer.
	*/
	void read();
	/**
	 Gets necessary information from header.
	*/
	void parse();
	/**
	  Sorts table of sections by raw_offset
	*/
	void sort();
	
	/**
	@return Return integer formed of @ref size bytes from the position pos.
	*/
	int get(int pos, int size=4);

	
	int number_of_sections;///<Number of sections in input file
	int base;///< Base of addresses in memory
	int entry_point;///< Entry point of input file
	entry* table;///< Table of sections
	string filename; ///<input file name
	unsigned char *data; ///<buffer containing binary file
	int dataSize; ///<size of buffer data
};
#endif