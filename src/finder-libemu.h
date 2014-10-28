#ifndef FINDER_LIBEMU_H
#define FINDER_LIBEMU_H

#include <iostream>
#include <fstream>

#include "finder.h" 
#include "timer.h"

namespace find_decryptor
{

using namespace std;

/**
  @brief
    Class finding instructions to emulate.
 */
class FinderLibemu : public Finder {
public:
	/**
	Constructor.
	*/
	FinderLibemu();
	/**
	Wrap on functions finding writes to memory and indirect jumps.
	*/
	int find();
};

} //namespace find_decryptor

#endif // FINDER_LIBEMU_H