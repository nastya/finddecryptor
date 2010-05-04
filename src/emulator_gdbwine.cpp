#include <csignal> 
#include <cstdlib>
#include <fstream>
#include "emulator_gdbwine.h"
#include "fdostream.h"

Emulator_GdbWine::Emulator_GdbWine()
{
	pid[0] = pid[1] = 0;
	out = NULL;
	dirty = false;
}
Emulator_GdbWine::~Emulator_GdbWine()
{
	stop();
}

void Emulator_GdbWine::start(PEReader* r) 
{
	int fd[3][2];
	pipe(fd[0]);
	pipe(fd[1]);
	pipe(fd[2]);
	
	stream_gdb(fd,r->name());
	stream_ctl(fd);
	stream_main(fd);
	reader = r;
}
void Emulator_GdbWine::stop()
{
	if (dirty) {
		(*out) << "kill" << endl;
	}
	for (int i=0; i<2; i++)
	{
		if (!pid[i]) continue;
		kill(pid[i], SIGKILL);
		pid[i] = 0;
	}
	if (out) delete out;
	out = NULL;
}

void Emulator_GdbWine::begin(int pos)
{
	if (dirty) {
		get_clean();
		(*out) << "kill" << endl;
		(*out) << "exec-file wine" << endl;
	}
	dirty = true;
	
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
	(*out) << "tb * 0x" << hex << reader->entrance() << endl;
	(*out) << "c" << endl;

	if (!get_clean()) {
		cerr	<< "Something is wrong with GdbWine emulator." << endl\
			<< "Make sure you have winecfg (or any other wine application) running." << endl
			<< "Also, 64-bit gdb is not supported (it can't debug 32-bit applications)." << endl;
		exit(0);
	}
	
	if (pos!=0) jump(pos);
}
void Emulator_GdbWine::jump(int pos)
{
	pos = reader->map(pos);
	(*out) << "tb * 0x" << hex << pos << endl;
	(*out) << "j * 0x" << hex << pos << endl;
}

bool Emulator_GdbWine::get_clean() {
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
bool Emulator_GdbWine::get_command(char *buff, int size)
{
	if (!get_clean()) return false;
	string str;
	int x;
	char *y = (char *) &x;

	for (int i=0; i<size; i+=2) {
		(*out) << "x/x $eip+" << i << endl;
		getline(cin,str);
		if (str[0]=='!') return false;
		str.replace(0,str.find(':'),"");
		x = str_to_int(str);
		buff[0+i] = y[0];
		buff[1+i] = y[1];
	}
	return true;
}
unsigned int Emulator_GdbWine::get_register(Register reg)
{
	get_clean();
	(*out) << "i r " << Registers[reg] << endl;
	string str;
	getline(cin,str);
	return str_to_int(str);
}
unsigned int Emulator_GdbWine::str_to_int(string str)
{
	unsigned int num;
	str.replace(0,str.find("0x"),"");
	sscanf(str.c_str(),"%x",&num);
	return num;
}
bool Emulator_GdbWine::step()
{
	get_clean();
	(*out) << "si" << endl;
	string str;
	getline(cin,str);
	return true;
}
void Emulator_GdbWine::fd_dup(int fd[3][2], int s0, int s1)
{
	dup2(fd[s0][0], 0);
	dup2(fd[s1][1], 1);
	fd_close(fd);
}
void Emulator_GdbWine::fd_close(int fd[3][2])
{
	close(fd[0][0]);
	close(fd[0][1]);
	close(fd[1][0]);
	close(fd[1][1]);
	close(fd[2][0]);
	close(fd[2][1]);
}
void Emulator_GdbWine::stream_main(int fd[3][2]) 
{
	dup2(fd[2][0], 0);
	out = new fdostream(fd[0][1]);
	close(fd[0][0]);
	close(fd[1][0]);
	close(fd[1][1]);
	close(fd[2][0]);
	close(fd[2][1]);
}
void Emulator_GdbWine::stream_gdb(int fd[3][2], string name) 
{
	pid[0] = fork();
	if (pid[0]) return;
	
	fd_dup(fd,0,1);
	dup2(1,2); /// We want to get cerr in cout.
	
	execlp("gdb","gdb","--quiet","--args","wine",name.c_str(),NULL);
}

/// Reading from gdb output with some post-processing
void Emulator_GdbWine::stream_ctl(int fd[3][2]) 
{
	pid[1] = fork();
	if (pid[1]) return;
	
	fd_dup(fd,1,2);
	
	string str;
//	ofstream log("../log/gdbwine.txt");
//	log.close();
	for(;;)
	{
		getline(cin,str);
		while ((str[0]=='(') && (str.length() >= 6)) 
		{
			str.replace(0,6,"");
		}
		if (str.length() == 0) continue;
//		log.open("../dbg.txt",ios_base::out|ios_base::app);
//		log << str << endl;
//		log.close();
		if (str.find("Cannot")!=string::npos)
		{
			str = "!Warning!";
		}
		else if ((str.find("Program received")!=string::npos) || 
			(str.find("Program exited")!=string::npos) || 
			(str.find("The program is not being run")!=string::npos) || 
			(str.find("No registers")!=string::npos))
		{
			str = "!Error!";
		}
		cout << str << endl;
	}
}
