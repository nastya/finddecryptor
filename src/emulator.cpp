#include "emulator.h"
#include <unistd.h>
#include <signal.h> 
#include <stdio.h>
#include <string.h>

#include <stdlib.h>


Emulator::Emulator()
{
	pid[0] = pid[1] = 0;
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
	
} 

void Emulator::end()
{
	cout << "kill" << endl;
	cout << "exec-file wine" << endl;
}
void Emulator::begin(int start, int pos)
{
	cout << "b * 0xb7fff424" << endl;
	cout << "run" << endl;
	cout << "d" << endl;
	cout << "b * 0xb7ea9c72" << endl;
	cout << "c" << endl;
	cout << "c 29" << endl;
	cout << "d" << endl;
	cout << "b * 0x"<< hex<<start<< endl;
	cout << "c" << endl;
	cout << "d" << endl;

	if (pos!=0) jump(pos);
}
void Emulator::jump(int pos)
{
	cout << "b * 0x" << hex << pos << endl;
	cout << "j * 0x" << hex << pos << endl;
	cout << "d" << endl;	
}
void Emulator::stream_main(int fd[3][2]) 
{
	set_dup(fd, 2, 0);
}
void Emulator::set_dup(int fd[3][2], int s0, int s1)
{
	dup2(fd[s0][0], 0);
	dup2(fd[s1][1], 1);
	close(fd[0][1]);
	close(fd[0][0]);
	close(fd[1][1]);
	close(fd[1][0]);
	close(fd[2][0]);
	close(fd[2][1]);
}

string Emulator::get_command_string(bool do_step)
{
	cout << "x/i $eip" << endl;
	if (do_step) step();
	string str = " ";
	while (str[0]!='=')
	{
		getline(cin,str);
		//cerr<<"\t"<<str<<endl;
		while ((str[0]=='(') && (str.length() > 6)) 
		{
			str.replace(0,6,"");
		}
	}
	str.replace(0,3,"");
	//cerr<<"\t"<<str<<endl;
	return str;
} 
int Emulator::get_register(Register reg)
{
	cout << "i r " << Registers[reg] << endl;
	string str;
	while (str[0]!=Registers[reg][0])
	{
		getline(cin,str);
		//cerr<<"\t"<<str<<endl;
		while ((str[0]=='(') && (str.length() > 6)) 
		{
			str.replace(0,6,"");
		}
	}
	for(int i=0;str.c_str()[i]!='\0';i++)
		if (str.c_str()[i]=='0'&&str.c_str()[i+1]=='x')
			return str_to_int(&str.c_str()[i+2]);
}
int Emulator::str_to_int(const char *str)
{
	int num;
	sscanf(str,"%X",&num); ///TODO: is it really %X we need to use? May be %x?
	//cerr << "0x" << hex << num << endl;
	return num;
}
void Emulator::step()
{
	cout << "si" << endl;	
}
void Emulator::stream_gdb(int fd[3][2], char* filename) 
{
	pid[0] = fork();
	if (pid[0]) return;
	
	set_dup(fd,0,1);
	
	execlp("gdb","gdb","-silent","--args","wine",filename,NULL);
}

void Emulator::stream_ctl(int fd[3][2]) 
{
	pid[1] = fork();
	if (pid[1]) return;
	
	set_dup(fd,1,2);
	
	char buf[1];
	char str[5000];
	int pos=0,i=0;
	for(;;)
	{
		while (read(0,buf,1))
		{
			str[pos]=buf[0];
			pos++;
			if((buf[0]==' ')||(buf[0]=='\n'))
				break;
		}
		str[pos]='\0';
		write(1,&str,pos);
		pos=0;
	}
}
