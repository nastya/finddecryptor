#include "finder.h"
#include "emulator_gdbwine.h"
#include "emulator_libemu.h"

using namespace std;

const Mode Finder::mode = MODE_32;
const Format Finder::format = FORMAT_INTEL;
const int Finder::maxBackward = 20;

Finder::Command::Command(int a, INSTRUCTION i) {
	addr = a;
	inst = i;
}

Finder::Finder(int type)
{
	switch (type) {
		case 1:
			emulator = new Emulator_LibEmu();
			break;
		case 0:
		default:
			emulator = new Emulator_GdbWine();
	}
	reader = NULL;
	regs_known = new bool[RegistersCount];
	regs_target = new bool[RegistersCount];
	log = NULL;
#ifdef FINDER_LOG
	log = new ofstream("../log/finder.txt");
#endif
	if (log) switch (type) {
		case 1:
			(*log) << "### Using LibEmu emulator. ###" << endl;
			break;
		case 0:
		default:
			(*log) << "### Using GdbWine emulator. ###" << endl;
	}
	Timer::start();
}

Finder::~Finder()
{
	Timer::stop();
	if (*log) 
	{
		(*log) << endl << endl;
		(*log) << "Time total: " << dec << Timer::secs() << " seconds." << endl;
		(*log) << "Time spent on load: " << dec << Timer::secs(TimeLoad) << " seconds." << endl;
		(*log) << "Time spent on find: " << dec << Timer::secs(TimeFind) << " seconds." << endl;
		(*log) << "Time spent on find_memory: " << dec << Timer::secs(TimeFindMemory) << " seconds." << endl;
		//(*log) << "Time spent on find_jump: " << dec << Timer::secs(TimeFindJump) << " seconds." << endl;
		(*log) << "Time spent on launches: " << dec << Timer::secs(TimeLaunches) << " seconds." << endl;
		(*log) << "Time spent on backwards traversal: " << dec << Timer::secs(TimeBackwardsTraversal) << " seconds." << endl;
		(*log) << "Time spent on emulator launches (total): " << dec << Timer::secs(TimeEmulatorStart) << " seconds." << endl;
	}
	
	delete[] regs_known;
	delete[] regs_target;
	if (log)
	{
		log->close();
		delete log;
	}
	delete emulator;
	delete reader;
}
void Finder::load(string name)
{
	Timer::start(TimeLoad);
	delete reader;
	reader = new Reader();
	reader->load(name);
	if (log) {
		(*log) << endl << "Loaded file \'" << name << "\"." << endl;
		(*log) << "File size: 0x" << hex << reader->size() << "." << endl << endl;
	}
	if (PEReader::is_of_type(reader))
	{
		reader = new PEReader(reader);
		(*log) << "Looks like a PE file." << endl << endl;
	}
	emulator->bind(reader);
	Timer::stop(TimeLoad);
}
void Finder::launch(int pos)
{
	Timer::start(TimeLaunches);
	if (log) (*log) << "Launching from position 0x" << hex << pos << endl;
	int a[1000]={0}, i,k,num,amount=0, barrier;
	bool flag = false;
	Command cycle[256];
	INSTRUCTION inst;
	Timer::start(TimeEmulatorStart);
	emulator->begin(pos);
	Timer::stop(TimeEmulatorStart);
	char buff[10] = {0};
	for (int strnum=0;;strnum++)
	{
		if (!emulator->get_command(buff)) {
			if (log) (*log) << " Execution error, stopping instance." << endl;
			Timer::stop(TimeLaunches);
			return;
		}
		num = emulator->get_register(EIP);
		get_instruction(&inst, (BYTE *) buff, mode);
		if (log) (*log) << "  Command: 0x" << hex << num << ": " << instruction_string(&inst, num) << endl;
		if (!emulator->step()) {
			if (log) (*log) << " Execution error, stopping instance." << endl;
			Timer::stop(TimeLaunches);
			return;			
		}
		get_operands(&inst);
		for (i=0;i<RegistersCount;i++)
		{
			if (regs_target[i]&&regs_known[i])
			{
				regs_target[i] = false;
			}
			else if (regs_target[i]&&!regs_known[i])
			{
				for (k=0;k<RegistersCount;k++)
				{
					if (!regs_target[k])
					{
						regs_target[k] = regs_known[k];
					} else {
						regs_known[k] = regs_target[k];
					}
				}
				check(&instructions_after_getpc);
				int em_start = backwards_traversal(pos_getpc);
				if (em_start<0)
				{
					if (log) (*log) <<  " Backwards traversal failed (nothing suitable found)." << endl;
					Timer::stop(TimeLaunches);
					return;
				}
				if (log) (*log) <<  " relaunch (because of " << Registers[i] << "). New position: 0x" << hex << em_start << endl;
				Timer::stop(TimeLaunches);
				return launch(em_start);
			}
		}
		memset(regs_target,false,RegistersCount);
		int kol = 0;
		for (i=0;i<amount;i++)
		{
			if (a[i]==num)
			{
				kol++;
			}
		}
		if (kol>=2)
		{
			int neednum = num;
			for (barrier=0;barrier<strnum+10;barrier++)
			{
				cycle[barrier] = Command(num,inst);
				if (!emulator->get_command(buff)) {
					if (log) (*log) << " Execution error, stopping instance." << endl;
					Timer::stop(TimeLaunches);
					return;
				}
				num = emulator->get_register(EIP);
				get_instruction(&inst, (BYTE *) buff, mode);
				if (log) (*log) << "  Command: 0x" << hex << num << ": " << instruction_string(&inst, num) << endl;
				if (!emulator->step()) {
					if (log) (*log) << " Execution error, stopping instance." << endl;
					Timer::stop(TimeLaunches);
					return;			
				}
				if (num==neednum)
				{
					flag = true;
					break;
				}
			}
		}
		if (flag) break;
		a[amount++]=num;
	}
	
	if (flag) {
		k = verify(cycle,barrier+1);

		if (log) {
			(*log) << " Cycle found: " << endl;
			for (i=0; i<=barrier; i++)
				(*log) << " 0x" << hex << cycle[i].addr << ":  " << instruction_string(&(cycle[i].inst), cycle[i].addr) << endl;
			if (k!=-1)
			{
				(*log) << " Indirect write in line #" << k << ", launched from position 0x" << hex << pos << endl;
			} else {
				(*log) << " No indirect writes." << endl;
			}
		}

		if (k!=-1)
		{
			cout << "Instruction \"" << instruction_string(pos_getpc) << "\" on position 0x" << hex << pos_getpc << "." << endl;
			cout << "Cycle found: " << endl;
			for (i=0; i<=barrier; i++)
				cout << " 0x" << hex << cycle[i].addr << ":  " << instruction_string(&(cycle[i].inst), cycle[i].addr) << endl;
			cout << " Indirect write in line #" << k << ", launched from position 0x" << hex << pos << endl;
#ifdef FINDER_ONCE
			Timer::stop(TimeLaunches);
			exit(0);
#endif
		}
	}
	Timer::stop(TimeLaunches);
}

int Finder::instruction(INSTRUCTION *inst, int pos) {
	return get_instruction(inst, reader->pointer() + pos, mode);
}
string Finder::instruction_string(INSTRUCTION *inst, int pos) {
	if (!inst->ptr) return "UNKNOWN";
	char str[256];
	get_instruction_string(inst, format, (DWORD)pos, str, sizeof(str));
	return (string) str;
}
string Finder::instruction_string(int pos) {
	INSTRUCTION inst;
	instruction(&inst, pos);
	return instruction_string(&inst,pos);
}
void Finder::find() 
{
	Timer::start(TimeFind);
	INSTRUCTION inst;
	for (int i=reader->start(); i<reader->size(); i++)
	{
		/// TODO: check opcodes
		switch (reader->pointer()[i])
		{
			/// fstenv: 0xf2d9, 0xd9
			case 0xf2:
				if ((reader->pointer()[i+1]) != 0xd9) continue; /// TODO: check if i+1 is present
			case 0xd9:
				break;
			/// call: 0xe8, 0xff, 0x9a
			case 0xe8:
			case 0xff:
			case 0x9a:
				break;
			default:
				continue;
		}
		int len = instruction(&inst, i);
		if (!len || (len + i > reader->size())) continue;
		switch (inst.type)
		{
			case INSTRUCTION_TYPE_FPU_CTRL:
				if (strcmp(inst.ptr->mnemonic,"fstenv")==0) break;
				continue;
			case INSTRUCTION_TYPE_CALL:
				if ((strcmp(inst.ptr->mnemonic,"call")==0) && (inst.op1.type==OPERAND_TYPE_IMMEDIATE)) break;
				continue;
			default:
				continue;
		}
		//cerr << "0x" << hex << i << ": 0x" << hex << (int) reader->pointer()[i] << " | 0x" << hex << (int) inst.opcode << " " << inst.ptr->mnemonic << endl;
		pos_getpc = i;
		if (log) (*log) << "Instruction \"" << instruction_string(i) << "\" on position 0x" << hex << i << "." << endl;
		find_memory(i);
		//find_jump(i);
		if (log) (*log) << "*********************************************************" << endl;
		instructions_after_getpc.clear();
	}
	Timer::stop(TimeFind);
}

void Finder::find_memory(int pos)
{
	Timer::start(TimeFindMemory);
	INSTRUCTION inst;
	int len;
	for (int p=pos; p<reader->size(); p+=len)
	{
		len = instruction(&inst,p);
		if (!len || (len + p > reader->size()))
		{
			p++;
			continue;
		}
		instructions_after_getpc.push_back(inst);
		if (!is_write_indirect(&inst)) continue;
		if (log) (*log) << "Write to memory detected: " << instruction_string(&inst,p) << " on position 0x" << hex << p << endl;
		if (start_positions.count(p))
		{
			if (log) (*log) << "Not running, already checked." << endl;
			Timer::stop(TimeFindMemory);
			return;
		}
		start_positions.insert(p);
		memset(regs_known,false,RegistersCount);
		memset(regs_target,false,RegistersCount);
		get_operands(&inst);
		check(&instructions_after_getpc);
		int em_start = backwards_traversal(pos);
		if (em_start<0)
		{
			if (log) (*log) <<  " Backwards traversal failed (nothing suitable found)." << endl;
			Timer::stop(TimeFindMemory);
			return;
		}
		print_commands(&instructions_after_getpc,1);
		launch(em_start);
		Timer::stop(TimeFindMemory);
		return;
	}
	Timer::stop(TimeFindMemory);
}

bool Finder::regs_closed() {
	for (int i=0; i<RegistersCount; i++)
		if (regs_target[i]) return false;
	return true;
}
void Finder::get_commands(vector <INSTRUCTION>* commands, vector <int>* num_commands, vector <int>* prev)
{
	commands->clear();
	INSTRUCTION inst;
	for (int i=num_commands->size()-1;i!=-1;i=(*prev)[i])
	{
		instruction(&inst,(*num_commands)[i]);
		commands->push_back(inst);
	}
}
void Finder::check(vector <INSTRUCTION>* instructions)
{
	for (int k=instructions->size()-1;k>=0;k--)
		check(&((*instructions)[k]));
}
void Finder::add_target(OPERAND *op) {
	if (MASK_FLAGS(op->flags)==F_f) return;
	switch (op->type)
	{
		case OPERAND_TYPE_REGISTER:
			regs_target[int_to_reg(op->reg)] = true;
			break;
		case OPERAND_TYPE_MEMORY:
			if (op->basereg == REG_NOP) break;
			regs_target[int_to_reg(op->basereg)] = true;
			break;
		default:;
	}
}
void Finder::get_operands(INSTRUCTION *inst)
{
	if (inst->type == INSTRUCTION_TYPE_LOOP) 
		regs_target[ECX] = true;
	if (inst->op1.type==OPERAND_TYPE_MEMORY)
		add_target(&(inst->op1));
	add_target(&(inst->op2));
	add_target(&(inst->op3));
}
void Finder::check(INSTRUCTION *inst)
{
	/// TODO: Add more instruction types here.
	/// TODO: WARNING: Check logic.
	int r;
	switch (inst->type)
	{
		case INSTRUCTION_TYPE_XOR:
		case INSTRUCTION_TYPE_SUB:
		case INSTRUCTION_TYPE_SBB:
		case INSTRUCTION_TYPE_DIV:
		case INSTRUCTION_TYPE_IDIV:
			if (inst->op1.type != OPERAND_TYPE_REGISTER) break;
			r = int_to_reg(inst->op1.reg);
			if ((inst->op1.type == inst->op2.type) && (inst->op1.reg == inst->op2.reg))
			{
				regs_target[r] = false;
				regs_known[r] = true;
				break;
			}
			if (regs_target[r]) add_target(&(inst->op2));
			break;
		case INSTRUCTION_TYPE_ADD:
		case INSTRUCTION_TYPE_AND:
		case INSTRUCTION_TYPE_OR:
		case INSTRUCTION_TYPE_MUL:
		case INSTRUCTION_TYPE_IMUL:
			if (inst->op1.type != OPERAND_TYPE_REGISTER) break;
			r = int_to_reg(inst->op1.reg);
			if (regs_target[r]) add_target(&(inst->op2));
			break;
		case INSTRUCTION_TYPE_MOV:
		case INSTRUCTION_TYPE_LEA:
			if (inst->op1.type != OPERAND_TYPE_REGISTER) break;
			r = int_to_reg(inst->op1.reg);
			regs_known[r] = true;
			if (regs_target[r])
			{
				regs_target[r] = false;
				add_target(&(inst->op2));
			}
			break;
		case INSTRUCTION_TYPE_POP:
			if (inst->op1.type != OPERAND_TYPE_REGISTER) break;
			r = int_to_reg(inst->op1.reg);
			regs_known[r] = true;
			if (regs_target[r])
			{
				regs_target[r] = false;
				regs_target[ESP] = true;
			}
			break;
		case INSTRUCTION_TYPE_FPU:
			if (strcmp(inst->ptr->mnemonic,"fptan")!=0) break; /// TODO: check this command.
		case INSTRUCTION_TYPE_FCMOVC:
		case INSTRUCTION_TYPE_CALL:
			regs_target[ESP] = false;
			regs_known[ESP] = true;
			//if (get_write_indirect(inst,&r))
			//{
			//	regs_target[r]=true;
			//	regs_known[r]=false;
			//}
			//if (inst->op1.type==OPERAND_TYPE_REGISTER)
			//{
			//	r = int_to_reg(inst->op1.reg);
			//	regs_target[r]=true;
			//	regs_known[r]=false;
			//}
			break;
		default:;
	}
}

void Finder::print_commands(vector <INSTRUCTION>* v, int start)
{
	int i=0;
	vector<INSTRUCTION>::iterator p;
	if (log) (*log) << " Commands in queue:"<< endl;
	for (vector<INSTRUCTION>::iterator p=v->begin(); p!=v->end(); i++,p++)
	{
		if (i<start) continue;
		if (log) (*log) << "  " << instruction_string(&(*p)) << endl;
	}
}

int Finder::backwards_traversal(int pos)
{
	if (regs_closed()) return pos;
	Timer::start(TimeBackwardsTraversal);
	INSTRUCTION inst;
	int length=1;
	bool regs_target_bak[RegistersCount], regs_known_bak[RegistersCount];
	memcpy(regs_target_bak,regs_target,RegistersCount);
	memcpy(regs_known_bak,regs_known,RegistersCount);
	vector <int> queue, prev;
	vector <INSTRUCTION> commands;
	queue.push_back(pos);
	prev.push_back(-1);
	for (int cur=0; (cur<length) && (cur<maxBackward); cur++)
	{
		for (int i=1; (i<=MaxCommandSize) && (i<=queue[cur]); i++)
		{
			if (instruction(&inst,queue[cur]-i)!=i) continue;
			queue.push_back(queue[cur]-i);
			prev.push_back(cur);
			length++;
			get_commands(&commands,&queue,&prev);
			check(&commands);
			if (regs_closed())
			{
				length = 0;
				break;
			}
			memcpy(regs_target,regs_target_bak,RegistersCount);
			memcpy(regs_known,regs_known_bak,RegistersCount);
		}
		/// TODO: We should also check all static jumps to this point.
	}
	Timer::stop(TimeBackwardsTraversal);
	return regs_closed() ? queue[queue.size()-1] : -1;
}

int Finder::verify(Command *cycle, int size)
{
	int reg;
	for (int i=0;i<size;i++)
		if (get_write_indirect(&(cycle[i].inst), &reg))
			if (verify_changing_reg(cycle, size, reg))
				return i+1;
	return -1;
}

bool Finder::verify_changing_reg(Command *cycle, int size, int reg)
{
	if (!reader->is_within_one_block(emulator->get_register((Register)reg),cycle[0].addr)) return false;
	for (int i=0;i<size;i++)
		if (	is_write(&(cycle[i].inst)) && 
			(cycle[i].inst.op1.type==OPERAND_TYPE_REGISTER) && 
			(reg==int_to_reg(cycle[i].inst.op1.reg))
			) return true;
	return false;
}
int Finder::int_to_reg(int code)
{
	switch (code) {
		case REG_EAX:
			return EAX;
		case REG_EBX:
			return EBX;
		case REG_ECX:
			return ECX;
		case REG_EDX:
			return EDX;
		case REG_EDI:
			return EDI;
		case REG_ESI:
			return ESI;
		case REG_ESP:
			return ESP;
		case REG_EBP:
			return EBP;
	}
	return -1;
}
bool Finder::get_write_indirect(INSTRUCTION *inst, int *reg)
{
	if (!is_write_indirect(inst)) return false;
	*reg = int_to_reg(inst->op1.basereg);
	return true;
}
bool Finder::is_write_indirect(INSTRUCTION *inst)
{
	return is_write(inst) && (inst->op1.type == OPERAND_TYPE_MEMORY) && (inst->op1.basereg != REG_NOP);
}
bool Finder::is_write(INSTRUCTION *inst)
{
	switch (inst->type) {
		case INSTRUCTION_TYPE_XOR:
		case INSTRUCTION_TYPE_SUB:
		case INSTRUCTION_TYPE_SBB:
		case INSTRUCTION_TYPE_DIV:
		case INSTRUCTION_TYPE_IDIV:
			
		case INSTRUCTION_TYPE_ADD:
		case INSTRUCTION_TYPE_AND:
		case INSTRUCTION_TYPE_OR:
		case INSTRUCTION_TYPE_MUL:
		case INSTRUCTION_TYPE_IMUL:
			
		case INSTRUCTION_TYPE_MOV:
		case INSTRUCTION_TYPE_POP:
			return true;
		default:
			return false;
	}
}

int Finder::find_jump(int pos)
{
	Timer::start(TimeFindJump);
	int len;
	for (; pos<reader->size(); pos+=len)
	{
		INSTRUCTION inst;
		len = instruction(&inst,pos);
		if (!len || (len + pos > reader->size()))
		{
			pos++;
			continue;
		}
		switch (inst.type)
		{
			/// TODO: check
			case INSTRUCTION_TYPE_JMP:
			case INSTRUCTION_TYPE_JMPC:
				if ((inst.op1.type!=OPERAND_TYPE_MEMORY) || (inst.op1.basereg==REG_NOP)) continue;
				if (log) (*log) << " Indirect jump detected: " << instruction_string(&inst) << " on position 0x" << hex << pos << endl;
				get_operands(&inst);
				Timer::stop(TimeFindJump);
				return pos;
			default:;
		}
	}
	Timer::stop(TimeFindJump);
	return -1;
}