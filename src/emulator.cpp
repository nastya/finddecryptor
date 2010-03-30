#include "emulator.h"
#include <unistd.h>
#include <signal.h> 

Emulator::Emulator()
{
	pid = 0;
	launch();
}
Emulator::~Emulator()
{
	destroy();
}
void Emulator::launch()
{
	pid = fork();
	if (!pid)
	{
		chdir("../bochs");
		execlp("./bochs-iodebug-nogui","./bochs-iodebug-nogui","-qf","win95.bochs",NULL);
	}
}
void Emulator::destroy()
{
	if (pid) {
		kill(pid,SIGTERM);
	}
}
