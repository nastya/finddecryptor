#include "finder.h"

/** @mainpage Description
 Nastya's coursework.
*/

/** 
 Function, running application.
 Makes an example of Finder class and uses it for finding necessary comand sequences.
 @param argc Parameter of command string. Definition not specified.
 @param argv One parameter - name of the input file.
 */
int main(int argc, char** argv)
{
	Finder finder(argv[1]);
	finder.find();
	return 0;
}
