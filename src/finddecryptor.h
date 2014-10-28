#ifndef FINDDECRYPTOR_H
#define FINDDECRYPTOR_H

#include <string>
#include <list>

namespace find_decryptor
{
	class Finder;
}

using namespace std;
using namespace find_decryptor;

class FindDecryptor {
public:
	FindDecryptor(int finderType = 0, int emulatorType = 1);
	~FindDecryptor();
	void load(string name, bool guessType=false);
	void link(const unsigned char *data, unsigned int dataSize, bool guessType=false);
	int find();
	int get_start_list(int max, int* list);
	list <int> get_start_list();
	int get_sizes_list(int max, int* list);
	list <int> get_sizes_list();

private:
	Finder *finder;
};
#endif //FINDDECRYPTOR_H
