#ifndef EMULATOR_GDBWINE_H
#define EMULATOR_GDBWINE_H

#include "emulator.h"

using namespace std;

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
	bool get_clean();
	unsigned int str_to_int(string str);
	
	void stream_gdb(int fd[3][2], string name);
	void stream_ctl(int fd[3][2]);
	void stream_main(int fd[3][2]);
	void fd_dup(int fd[3][2], int s0, int s1);
	void fd_close(int fd[3][2]);	

	PEReader *reader;
	int pid[2];
	ostream *out;
	bool dirty;
};

#endif 
