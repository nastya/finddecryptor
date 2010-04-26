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
	void end();
	
	void jump(int pos);
	void step();
	string get_command_string(bool do_step = true);
	int get_register(Register reg);
private:
	int str_to_int(const char *str);
	
	void stream_gdb(int fd[3][2], char* filename);
	void stream_ctl(int fd[3][2]);
	void stream_main(int fd[3][2]);
	void set_dup(int fd[3][2], int s0, int s1);	
	int pid[2];	
};

#endif 
