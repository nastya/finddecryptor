#include "finder.h"

/** @mainpage Description
This program is implementation of algorythm proposed by Qinghua Zhang, Douglas S. Reeves, Peng Ning, S. Purushothaman Iyer in the article "Analyzing Network Trafﬁc To Detect Self-Decrypting Exploit Code".
They describe a method for detecting self-decrypting exploit codes. This method scans network traﬃc for the presence of a decryption routine, which is characteristic of such exploits. The proposed method uses static analysis and emulated instruction execution techniques. This improves the accuracy of determining the starting location and instructions of the decryption routine, even if self-modifying code is used. 
*/

/** 
 Function, running application.
 Makes an example of Finder class and uses it for finding necessary comand sequences.
 @param argc Parameter of command string. Definition not specified.
 @param argv One parameter - name of the input file.
 */
int main(int argc, char** argv)
{
	int type = 0;
	switch (argc)
	{
		case 2:
			break;
		case 3:
			if (strcmp(argv[2],"LibEmu")==0)
				type = 1;
			else if (strcmp(argv[2],"GdbWine")!=0)
			{
				cerr << "Unknown emulator type." << endl;
				return 0;
			}
			break;
		default:
			cerr << "Wrong usage." << endl;
			return 0;
	}
	Finder finder(argv[1],type);
	finder.find();
	return 0;
}
