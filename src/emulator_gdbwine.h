#ifndef EMULATOR_GDBWINE_H
#define EMULATOR_GDBWINE_H

#include "emulator.h"

namespace find_decryptor
{

using namespace std;

/**
	@brief
	Emulation via gdb+wine
*/

class Emulator_GdbWine : public Emulator {
public:
	Emulator_GdbWine();
	~Emulator_GdbWine();
	void stop();
	void begin(uint pos=0);
	
	bool step();
	bool get_command(char *buff, uint size=10);
	bool get_memory(char *buff, int addr, uint size=1);
	unsigned int get_register(Register reg);
private:
	/**
	  Continues emulation from the spesified position.
	  @param pos Spesified position.
	*/
	void jump(uint pos);
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
	void stream_gdb(int fd[3][2]);
	/**
	  Runs control process reading information from gdb and writing to main process. 
	  @param fd file descriptors used to transfer information between processes.
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

	int pid[2]; ///<Process identificators of main, control and gdb processes
	ostream *out; ///< Stream copy of fd[0][1] - fd used to write information to gdb
	bool dirty;///< a flag meaning that emulation has already started 
};

} //namespace find_decryptor

#endif 
