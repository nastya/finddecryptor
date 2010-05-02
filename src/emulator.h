#ifndef EMULATOR_H
#define EMULATOR_H

#include <iostream>
#include <cstring>
#include "data.h"

using namespace std;

class Emulator: Data{
public:	
	Emulator();
	~Emulator();
	void start(char* filename);
	void stop();
	void begin(int start,int pos=0);
	
	void jump(int pos);
	void step();
	bool get_command(char *buff, int size=10);
	unsigned int get_register(Register reg);
private:
	bool get_clean();
	unsigned int str_to_int(string str);
	
	void stream_gdb(int fd[3][2], char* filename);
	void stream_ctl(int fd[3][2]);
	void stream_main(int fd[3][2]);
	void fd_dup(int fd[3][2], int s0, int s1);
	void fd_close(int fd[3][2]);	
	int pid[2];
	ostream *out;
	bool dirty;
};

#endif 
