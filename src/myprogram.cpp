#include "finder.h"

/** @mainpage Description
This program is implementation of algorythm proposed by Qinghua Zhang, Douglas S. Reeves, Peng Ning, S. Purushothaman Iyer in the article "Analyzing Network Traffic To Detect Self-Decrypting Exploit Code".
They describe a method for detecting self-decrypting exploit codes. This method scans network traffic for the presence of a decryption routine, which is characteristic of such exploits. The proposed method uses static analysis and emulated instruction execution techniques. This improves the accuracy of determining the starting location and instructions of the decryption routine, even if self-modifying code is used. 
*/

/** 
 Function, running application.
 Makes an example of Finder class and uses it for finding necessary comand sequences.
 @param argc Parameter of command string. Definition not specified.
 @param argv One parameter - name of the input file.
 */
int main(int argc, char** argv)
{
	int type = 1;
	switch (argc) {
		case 2:
			break;
		case 3:
			if (strcmp(argv[2],"GdbWine") == 0) {
				type = 0;
			} else if (strcmp(argv[2],"LibEmu") == 0) {
				type = 1;
			} else if (strcmp(argv[2],"Qemu") == 0) {
				type = 2;
			} else {
				cerr << "Unknown emulator type." << endl;
				return 0;
			}
			break;
		default:
			cerr << "Wrong usage." << endl;
			return 0;
	}
	Finder finder(type);
	finder.load(argv[1], true);
	finder.find();
	exit(0); /// Hack for qemu. TODO: fix
	return 0;
}
