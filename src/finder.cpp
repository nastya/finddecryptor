#include "finder.h"

using namespace std;

const Mode Finder::mode = MODE_32;
const Format Finder::format = FORMAT_INTEL;

Command::Command(int a, INSTRUCTION i) {
	addr = a;
	inst = i;
}

Finder::Finder()
{
	init();
}

Finder::Finder(char* name)
{
	init();
	filename=name;
	emulator.start(filename);
	read_file(filename);
	reader.init(data);
	//reader.print_table();
}
Finder::~Finder()
{
	clear();
	delete[] regs_known;
	delete[] regs_target;
	if (log)
	{
		log->close();
		delete log;
	}
}
void Finder::init()
{
	regs_known = new bool[RegistersCount];
	regs_target = new bool[RegistersCount];
	data = NULL;
	dataSize = 0;
	log = NULL;
#ifdef FINDER_LOG
	log = new ofstream("../log.txt");
#endif
}
void Finder::clear()
{
	delete[] data;
	data = NULL;
	dataSize = 0;
}
void Finder::launch(int start, int pos)
{
	if (log) (*log) << "launch (0x" << hex << start << ", 0x" << hex << pos << ")" << endl;
	int a[1000]={0}, i,k,num,amount=0, barrier;
	bool flag = false;
	Command cycle[256];
	INSTRUCTION inst;
	emulator.begin(start,pos);
	char buff[10] = {0};
	for (int strnum=0;;strnum++)
	{
		if (!emulator.get_command(buff)) { /// Die on error, it will get SIGSEGV anyway.
			if (log) (*log) << " Execution error, stopping instance." << endl;
			return;
		}
		num = emulator.get_register(EIP);
		get_instruction(&inst, (BYTE *) buff, mode);
		emulator.step();
		if (log) (*log) << "  Command: 0x" << hex << num << ": " << instruction_string(&inst, num) << endl;
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
					return;
				}
				int startn = reader.get_starting_point(), posn = reader.calculate_virt_addr(em_start);
				if (log) (*log) <<  " relaunch (because of " << Registers[i] << ") 0x" << hex << startn << " 0x" << hex << posn << endl;
				return launch(startn,posn);
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
				if (!emulator.get_command(buff)) { /// Die on error, it will get SIGSEGV anyway.
					if (log) (*log) << " Execution error, stopping instance." << endl;
					return;
				}
				num = emulator.get_register(EIP);
				get_instruction(&inst, (BYTE *) buff, mode);
				emulator.step();
				if (log) (*log) << "  Command: 0x" << hex << num << ": " << instruction_string(&inst, num) << endl;
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
				(*log) << " Indirect write in line #" << k << ", launched: (0x" << hex << start << ", 0x" << hex << pos << ")" << endl;
			} else {
				(*log) << " No indirect writes." << endl;
			}
		}

		if (k!=-1)
		{
			cout << "Instruction \"" << instruction_string(pos_getpc) << "\" on position " << dec << pos_getpc << "." << endl;
			cout << "Cycle found: " << endl;
			for (i=0; i<=barrier; i++)
				cout << " 0x" << hex << cycle[i].addr << ":  " << instruction_string(&(cycle[i].inst), cycle[i].addr) << endl;
			cout << "Indirect write in line #" << k << ", launched: (0x" << hex << start << ", 0x" << hex << pos << ")" << endl;
			exit(0);
		}
	}
}

string Finder::instruction_string(INSTRUCTION *inst, int pos) {
	char str[256];
	get_instruction_string(inst, format, (DWORD)pos, str, sizeof(str));
	return (string) str;
}
string Finder::instruction_string(int pos) {
	INSTRUCTION inst;
	get_instruction(&inst, &data[pos], mode);
	return instruction_string(&inst,pos);
}
void Finder::find() 
{
	INSTRUCTION inst;
	for (int i=0; i<dataSize; i++)
	{
		int len = get_instruction(&inst, &data[i], mode);		
		if (!len || (len + i > dataSize)) continue;
		if (strcmp(inst.ptr->mnemonic,"call")==0||strcmp(inst.ptr->mnemonic,"fstenv")==0||strcmp(inst.ptr->mnemonic,"fnstenv")==0)
		{
			pos_getpc = i;
			if (log) (*log) << "Instruction \"" << instruction_string(i) << "\" on position " << dec << i << "." << endl;
			find_memory(i);
			find_jump(i);
			if (log) (*log) << "*********************************************************" << endl;
		}
		instructions_after_getpc.clear();	
	}
}

void Finder::find_memory(int pos)
{
	int len;
	for (int p=pos; p<dataSize; p+=len)
	{
		INSTRUCTION inst;
		len = get_instruction(&inst,&data[p],mode);
		if (!len || (len + p > dataSize))
		{
			p++;
			continue;
		}
		instructions_after_getpc.push_back(inst);
		if (!is_write_indirect(&inst)) continue;
		if (log) (*log) << "Write to memory detected: " << instruction_string(&inst,p) << " on position " << p << endl;
		if (start_positions.count(p))
		{
			if (log) (*log) << "Not running, already checked." << endl;
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
			return;
		}
		print_commands(&instructions_after_getpc,1);
		launch(reader.get_starting_point(),reader.calculate_virt_addr(em_start));
		return;
	}
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
		get_instruction(&inst,&data[(*num_commands)[i]],mode);
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
			regs_target[ESP] = false;
			regs_known[ESP] = true;
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
	INSTRUCTION inst;
	int length=1;
	bool regs_target_bak[RegistersCount], regs_known_bak[RegistersCount];
	memcpy(regs_target_bak,regs_target,RegistersCount);
	memcpy(regs_known_bak,regs_known,RegistersCount);
	vector <int> queue, prev;
	vector <INSTRUCTION> commands;
	queue.push_back(pos);
	prev.push_back(-1);
	for (int cur=0; cur<length; cur++)
	{
		for (int i=1; i<=MaxCommandSize; i++)
		{
			if (get_instruction(&inst,&data[queue[cur]-i],mode)!=i) continue;
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
	if (!reader.is_within_one_block(emulator.get_register((Register)reg),cycle[0].addr)) return false;
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
	int len;
	for (; pos<dataSize; pos+=len)
	{
		INSTRUCTION inst;
		len = get_instruction(&inst,&data[pos],mode);
		if (!len || (len + pos > dataSize))
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
				if (log) (*log) << " Indirect jump detected: " << instruction_string(&inst) << " on position " << dec << pos << endl;
				get_operands(&inst);
				return pos;
			default:;
		}
	}
	return -1;
}

void Finder::read_file(char *name)
{
	clear();
	ifstream s(name);
	if (!s.good() || s.eof() || !s.is_open()) 
	{
		if (log) (*log) << "Error opening file!\n";
		return;
	}
	s.seekg(0, ios_base::beg);
	ifstream::pos_type begin_pos = s.tellg();
	s.seekg(0, ios_base::end);
	dataSize = static_cast<int>(s.tellg() - begin_pos);
	s.seekg(0, ios_base::beg);
	data = new unsigned char[dataSize];
	s.read((char *) data,dataSize);
	s.close();
}