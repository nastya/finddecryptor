#ifndef EMULATOR_H
#define EMULATOR_H

class Emulator {
public:
	Emulator();
	~Emulator();
	void launch();
	void destroy();
private:
	int pid;
};

#endif