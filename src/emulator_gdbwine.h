#ifndef EMULATOR_GDBWINE_H
#define EMULATOR_GDBWINE_H

#include "emulator.h"

using namespace std;

/**
	@brief
	Emulation via gdb+wine
*/

class Emulator_GdbWine : public Emulator {
public:	
	Emulator_GdbWine();
	~Emulator_GdbWine();
	void start(PEReader *r);
	void stop();
	void begin(int pos=0);
	
	void jump(int pos);
	bool step();
	bool get_command(char *buff, int size=10);
	unsigned int get_register(Register reg);
private:
	/**
	  Removes service information from the gdb's output.
	  @return Returns false if any error happened.
	*/
	bool get_clean();
	/**
	  @return Returns hex number stored in parameter @ref str. 
	*/
	unsigned int str_to_int(string str);
	
	/**
	  Runs process gbd.
	  @param fd file descriptors used to transfer information between processes.
	*/
	void stream_gdb(int fd[3][2], string name);
	/**
	  Runs control process reading information from gdb and writing to main process. 
	  @param fd file descriptors used to transfer information between processes.
	  @param name Name of input file - file to trace.
	*/
	void stream_ctl(int fd[3][2]);
	/**
	  Runs main process writing information to gdb and reading from control process.
	  @param fd file descriptors used to transfer information between processes.
	*/
	void stream_main(int fd[3][2]);
	/**
	  Makes pipe between a pair of processes of main, control and gdb.
	  @param fd file descriptors used to transfer information between processes.
	  @param s0 number of first process in pipe.
	  @param s1 number of second process in pipe.
	*/
	void fd_dup(int fd[3][2], int s0, int s1);
	/**
	  Closes all file descriptors.
	  @param fd file descriptors used to transfer information between processes.
	*/
	void fd_close(int fd[3][2]);	

	PEReader *reader;///<Pointer to an examplar of class PEReader which is used for taking special information out of PE-header.
	int pid[2]; ///<Process identificators of main, control and gdb processes
	ostream *out; ///< Stream copy of fd[0][1] - fd used to write information to gdb
	bool dirty;///< a flag meaning that emulation has already started 
};

#endif 
