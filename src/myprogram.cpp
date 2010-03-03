#include "finder.h"

int main(int argc,char** argv)
{
	Finder finder(argv[1]);
	finder.find();
	return 0;
}
