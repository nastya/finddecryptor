#include "emulator.h"
#include <unistd.h>
#include <signal.h> 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>  
#include <fcntl.h> 
#include <fstream>
#include "fdostream.h"

Emulator::Emulator()
{
	pid[0] = pid[1] = 0;
	out = NULL;
}
Emulator::~Emulator()
{
	stop();
}

void Emulator::start(char* filename) 
{
	int fd[3][2];
	pipe(fd[0]);
	pipe(fd[1]);
	pipe(fd[2]);
	
	stream_gdb(fd,filename);
	stream_ctl(fd);
	stream_main(fd);
}
void Emulator::stop()
{
	for (int i=0; i<2; i++)
	{
		if (!pid[i]) continue;
		kill(pid[i], SIGKILL);
		pid[i] = 0;
	}
	if (out) delete out;
} 

void Emulator::end()
{
	(*out) << "kill" << endl;
	(*out) << "exec-file wine" << endl;
}
void Emulator::begin(int start, int pos)
{
	/// Ok, with this method wine should be launched before starting the program.
	/// You can, for example, lauch winecfg, and keep it open while running the program.
	
	(*out) << "tc syscall set_tid_address" << endl;
	(*out) << "run" << endl;
	(*out) << "tc syscall set_thread_area" << endl;
	(*out) << "c" << endl;
	(*out) << "tc syscall rt_sigaction" << endl;
	(*out) << "c" << endl;
	(*out) << "catch syscall open" << endl;
	(*out) << "c" << endl;
	(*out) << "c 29" << endl;
	(*out) << "d" << endl;
	(*out) << "tb * 0x" << hex << start << endl;
	(*out) << "c" << endl;

	if (pos!=0) jump(pos);
}
void Emulator::jump(int pos)
{
	(*out) << "tb * 0x" << hex << pos << endl;
	(*out) << "j * 0x" << hex << pos << endl;
}

bool Emulator::get_clean() {
	string str = " ";
	bool ok = true;
	/// TODO: rework this? We need to scan until input stops and buffer ends.
	(*out) << "show prompt" << endl;
	while (str[0]!='G')
	{
		getline(cin,str);
		if (str[0]=='!') ok = false;
	}
	return ok;
}
bool Emulator::get_command(char *buff, int size)
{
	if (!get_clean()) return false;
	string str;
	int x;
	char *y = (char *) &x;

	for (int i=0; i<size; i+=2) {
		(*out) << "x/hex $eip+" << i << endl;
		getline(cin,str);
		if (str[0]=='!') return false;
		str.replace(0,str.find(':'),"");
		x = str_to_int(str);
		buff[0+i] = y[0];
		buff[1+i] = y[1];
	}
	return true;
}
int Emulator::get_register(Register reg)
{
	if (!get_clean()) return -1;
	(*out) << "i r " << Registers[reg] << endl;
	string str;
	getline(cin,str);
	return str_to_int(str);
}
int Emulator::str_to_int(string str)
{
	int num;
	str.replace(0,str.find("0x"),"");
	sscanf(str.c_str(),"%x",&num);
	return num;
}
void Emulator::step()
{
	(*out) << "si" << endl;
}
void Emulator::fd_dup(int fd[3][2], int s0, int s1)
{
	dup2(fd[s0][0], 0);
	dup2(fd[s1][1], 1);
	fd_close(fd);
}
void Emulator::fd_close(int fd[3][2])
{
	close(fd[0][0]);
	close(fd[0][1]);
	close(fd[1][0]);
	close(fd[1][1]);
	close(fd[2][0]);
	close(fd[2][1]);
}
void Emulator::stream_main(int fd[3][2]) 
{
	dup2(fd[2][0], 0);
	out = new fdostream(fcntl(fd[0][1],F_DUPFD,0));
	fd_close(fd);
}
void Emulator::stream_gdb(int fd[3][2], char* filename) 
{
	pid[0] = fork();
	if (pid[0]) return;
	
	fd_dup(fd,0,1);
	dup2(1,2); /// We want to get cerr in cout.
	
	execlp("gdb","gdb","--quiet","--args","wine",filename,NULL);
}

/// Reading from gdb output with some post-processing
void Emulator::stream_ctl(int fd[3][2]) 
{
	pid[1] = fork();
	if (pid[1]) return;
	
	fd_dup(fd,1,2);
	
	string str;
	//ofstream log("../dbg.txt");
	//log.close();
	for(;;)
	{
		getline(cin,str);
		while ((str[0]=='(') && (str.length() >= 6)) 
		{
			str.replace(0,6,"");
		}
		if (str.length() == 0) continue;
		//log.open("../dbg.txt",ios_base::out|ios_base::app);
		//log << str << endl;
		//log.close();
		if ((str.find("Cannot")!=string::npos)||(str.find("Program received")!=string::npos)) {
			str = "!Error!";
		}
		cout.write(str.c_str(),str.length()); /// We don't want to pass \0 to gdb.
		cout << endl;
	}
}
