#ifndef FINDDECRYPTOR_H
#define FINDDECRYPTOR_H

#include <string>

using namespace std;

class Finder;

class FindDecryptor {
public:
	FindDecryptor(int finderType = 0, int emulatorType = 1);
	~FindDecryptor();
	void load(string name, bool guessType=false);
	void link(const unsigned char *data, unsigned int dataSize, bool guessType=false);
	int find();
	
private:
	Finder *finder;
};
#endif //FINDDECRYPTOR_H
