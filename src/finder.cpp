#include "finder.h"
#include "emulator_gdbwine.h"
#include "emulator_libemu.h"
#include "emulator_qemu.h"

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
		case 2:
			emulator = new Emulator_Qemu();
			break;
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
		(*log) << "Time spent on find_memory_and_jump: " << dec << Timer::secs(TimeFindMemoryAndJump) << " seconds." << endl;
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
void Finder::load(string name, bool guessType) {
	Timer::start(TimeLoad);
	Reader *reader = new Reader();
	reader->load(name);
	if (log) {
		(*log) << endl << "Loaded file \'" << name << "\"." << endl;
		(*log) << "File size: 0x" << hex << reader->size() << "." << endl << endl;
	}
	apply_reader(reader, guessType);
	Timer::stop(TimeLoad);
}
void Finder::link(const unsigned char *data, uint dataSize, bool guessType) {
	Timer::start(TimeLoad);
	Reader *reader = new Reader();
	reader->link(data, dataSize);
	if (log) {
		(*log) << endl << "Loaded data at 0x" << hex << data << "\"." << endl;
		(*log) << "Data size: 0x" << hex << reader->size() << "." << endl << endl;
	}
	apply_reader(reader, guessType);
	Timer::stop(TimeLoad);
}
void Finder::apply_reader(Reader *reader, bool guessType) {
	if (guessType) {
		if (Reader_PE::is_of_type(reader)) {
			reader = new Reader_PE(reader);
			if (log) (*log) << "Looks like a PE file." << endl << endl;
		}
	}
	delete this->reader;
	this->reader = reader;
	emulator->bind(reader);
}
void Finder::launch(int pos)
{
	Timer::start(TimeLaunches);
	if (log) (*log) << "Launching from position 0x" << hex << pos << endl;
	int a[1000]={0}, i,k,num,amount=0, barrier;
	bool flag = false;
//	Command cycle[256];
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
		if (!reader->is_valid(num)) {
			if (log) (*log) << " Reached end of the memory block, stopping instance." << endl;
			Timer::stop(TimeLaunches);
			return;
		}
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
		if ((uint)strnum >= instructions_after_getpc.size() + am_back)
			instructions_after_getpc.push_back(inst);
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
		if (is_write_indirect(&inst))
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
			matches++;
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
int Finder::find() {
	matches = 0;
	Timer::start(TimeFind);
	INSTRUCTION inst;
	for (uint i=reader->start(); i<reader->size(); i++)
	{
		/// TODO: check opcodes
		switch (reader->pointer()[i])
		{
			/// fsave/fnsave: 0x9bdd, 0xdd
			case 0x9b:
				if ((reader->pointer()[i+1]) != 0xd9) continue; /// TODO: check if i+1 is present
			case 0xdd:
				break;
			/// fstenv/fnstenv: 0xf2d9, 0xd9
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
		uint len = instruction(&inst, i);
		if (!len || (len + i > reader->size())) continue;
		switch (inst.type)
		{
			case INSTRUCTION_TYPE_FPU_CTRL:
				if (strcmp(inst.ptr->mnemonic,"fstenv")==0) break;
				if (strcmp(inst.ptr->mnemonic,"fsave")==0) break;
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
		find_memory_and_jump(i);
		//find_jump(i);
		if (log) (*log) << "*********************************************************" << endl;
		instructions_after_getpc.clear();
	}
	Timer::stop(TimeFind);
	return matches;
}

void Finder::find_memory_and_jump(int pos)
{
	Timer::start(TimeFindMemoryAndJump);
	INSTRUCTION inst;
	uint len;
	set<uint> nofollow;
	for (uint p=pos; p<reader->size(); p+=len)
	{
		len = instruction(&inst,p);
		if (!len || (len + p > reader->size()))
		{
			if (log) (*log) <<  " Dissasembling failed." << endl;
			Timer::stop(TimeFindMemoryAndJump);
			return;
		}
		if (log) (*log) << "Instruction: " << instruction_string(&inst,p) << " on position 0x" << hex << p << endl;
		instructions_after_getpc.push_back(inst);
		switch (inst.type)
		{
			/// TODO: check
			case INSTRUCTION_TYPE_JMP:
			case INSTRUCTION_TYPE_JMPC:
				if ((inst.op1.type==OPERAND_TYPE_MEMORY) && (inst.op1.basereg!=REG_NOP)) {
					if (log) (*log) << " Indirect jump detected: " << instruction_string(&inst) << " on position 0x" << hex << p << endl;
					get_operands(&inst);
				}
				if ((!nofollow.count(p)) && (strcmp(inst.ptr->mnemonic,"jmp")==0) && (inst.op1.type==OPERAND_TYPE_IMMEDIATE)) {
					nofollow.insert(p);
					p += inst.op1.immediate;
					continue;
				}
				break;
			case INSTRUCTION_TYPE_CALL:
				if ((!nofollow.count(p)) && (strcmp(inst.ptr->mnemonic,"call")==0) && (inst.op1.type==OPERAND_TYPE_IMMEDIATE)) {
					nofollow.insert(p);
					p += inst.op1.immediate;
					continue;
				}
				break;
			default:;
		}
		if (!is_write_indirect(&inst)) continue;
		if (log) (*log) << "Write to memory detected: " << instruction_string(&inst,p) << " on position 0x" << hex << p << endl;
		if (start_positions.count(p))
		{
			if (log) (*log) << "Not running, already checked." << endl;
			Timer::stop(TimeFindMemoryAndJump);
			return;
		}
		start_positions.insert(p);
		memset(regs_known,false,RegistersCount);
		memset(regs_target,false,RegistersCount);
		get_operands(&inst);
		check(&instructions_after_getpc);
		int em_start = backwards_traversal(pos_getpc);
		if (em_start<0)
		{
			if (log) (*log) <<  " Backwards traversal failed (nothing suitable found)." << endl;
			Timer::stop(TimeFindMemoryAndJump);
			return;
		}
		print_commands(&instructions_after_getpc,1);
		launch(em_start);
		Timer::stop(TimeFindMemoryAndJump);
		return;
	}
	Timer::stop(TimeFindMemoryAndJump);
}

bool Finder::regs_closed() {
	for (int i=0; i<RegistersCount; i++)
		if (regs_target[i]) return false;
	return true;
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
			if (op->reg == REG_ESP) break;
			regs_target[int_to_reg(op->reg)] = true;
			break;
		case OPERAND_TYPE_MEMORY:
			if (op->basereg == REG_NOP || op->reg == REG_ESP) break;
			regs_target[int_to_reg(op->basereg)] = true;
			break;
		default:;
	}
}
void Finder::get_operands(INSTRUCTION *inst)
{
	if (inst->type == INSTRUCTION_TYPE_LODS)
		regs_target[ESI] = true;
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
		case INSTRUCTION_TYPE_LODS:
			regs_known[EAX] = true;
			regs_target[EAX] = false;
			regs_target[ESI] = true;
			break;
		case INSTRUCTION_TYPE_STOS:
			regs_target[EAX] = true;
			regs_target[EDI] = true;
			break;
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
			regs_target[r] = false;
			regs_target[ESP] = true;
			break;
		case INSTRUCTION_TYPE_PUSH: /// TODO: check operands
			regs_target[ESP] = false;
			break;
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
		case INSTRUCTION_TYPE_OTHER:
			if (strcmp(inst->ptr->mnemonic,"cpuid")==0)
			{
				regs_target[EAX] = true;
				regs_target[EBX] = false;
				regs_target[ECX] = false;
				regs_target[EDX] = false;
				regs_known[EAX] = false;
				regs_known[EBX] = true;
				regs_known[ECX] = true;
				regs_known[EDX] = true;
			}
			break;
		case INSTRUCTION_TYPE_FPU_CTRL:
			if (strcmp(inst->ptr->mnemonic,"fstenv")==0) {
				add_target(&(inst->op1));
				if (regs_target[ESP]) { // If we need ESP. Else, use general fpu instuction logic.
					regs_target[ESP] = false;
					regs_known[ESP] = true;
					regs_target[HASFPU] = true;
					break;
				}
			} // No break here, going to default processing.
		default:
			if (MASK_EXT(inst->flags) == EXT_CP) { // Co-processor: FPU instructions
				regs_target[HASFPU] = false;
				regs_known[HASFPU] = true;
			};
			break;
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
	bool regs_target_bak[RegistersCount], regs_known_bak[RegistersCount];
	memcpy(regs_target_bak,regs_target,RegistersCount);
	memcpy(regs_known_bak,regs_known,RegistersCount);
	vector <int> queue[2];
	map<int,INSTRUCTION> instructions;
	INSTRUCTION inst;
	queue[0].push_back(pos);
	int m = 0;
	vector <INSTRUCTION> commands;
	for (int n=0; n<maxBackward; n++)
	{
		queue[m^1].clear();
		for (vector<int>::iterator p=queue[m].begin(); p!=queue[m].end(); p++)
		{
			for (int i=1; (i<=MaxCommandSize) && (i<=(*p)); i++)
			{
				int curr = (*p) - i;
				bool ok = false;
				int len = instruction(&inst,curr);
				switch (inst.type)
				{
					case INSTRUCTION_TYPE_JMP:
					case INSTRUCTION_TYPE_JMPC:
					case INSTRUCTION_TYPE_JECXZ:
						ok = (i == (inst.op1.immediate + len));
						break;
					default:;
				}
				if (len!=i && !ok) continue;
				instructions[curr] = inst;
				queue[m^1].push_back((*p)-i);
				commands.clear();
				for (int j=curr,k=0; k<=n; k++)
				{
					commands.push_back(instructions[j]);
					j += instructions[j].length;
				}
				check(&commands);
				am_back = commands.size();
				bool ret = regs_closed();
/*				if (log)
				{
					(*log) << "   BACKWARDS TRAVERSAL ITERATION" << endl;
					print_commands(&commands, 0);
					if (ret)
						(*log) << "Backwards traversal iteration succeeded." << endl;
					else
					{
						(*log) << "Backwards traversal iteration failed. Unknown registers: ";
						for (int i=0; i<RegistersCount; i++)
							if (regs_target[i]) (*log) << " " << dec << i;
						(*log) << endl;
					}
				} */
				memcpy(regs_target,regs_target_bak,RegistersCount);
				memcpy(regs_known,regs_known_bak,RegistersCount);
				if (ret) {
					Timer::stop(TimeBackwardsTraversal);
					return curr;
				}
			}
		}
		m ^= 1;
		/// TODO: We should also check all static jumps to this point.
	}
	Timer::stop(TimeBackwardsTraversal);
	return -1;
}
int Finder::verify(Command *cycle, int size)
{
	for (int i=0;i<size;i++)
		if (is_write_indirect(&(cycle[i].inst)))
			if (verify_changing_reg(&(cycle[i].inst), cycle, size))
				return i+1;
	return -1;
}

bool Finder::verify_changing_reg(INSTRUCTION *inst, Command *cycle, int size)
{
	int	mem  = inst->op1.displacement,
		reg0 = inst->op1.basereg,
		reg1 = inst->op1.reg,
		reg2 = inst->op1.indexreg;
	if (reg0 != REG_NOP) mem += emulator->get_register((Register) int_to_reg(reg0));
	if (reg1 != REG_NOP) mem += emulator->get_register((Register) int_to_reg(reg1));
	if (reg2 != REG_NOP) mem += emulator->get_register((Register) int_to_reg(reg2));
	if (inst->type == INSTRUCTION_TYPE_STOS)
	{
		reg0 = REG_EDI;
		mem = emulator->get_register((Register) int_to_reg(reg0));
	}
	if ((mem==0) || !reader->is_within_one_block(mem,cycle[0].addr)) return false;
	for (int i=0;i<size;i++) {
		if (	is_write(&(cycle[i].inst)) && 
			(cycle[i].inst.op1.type==OPERAND_TYPE_REGISTER) && 
			(
				((reg0!=REG_NOP) && (reg0==cycle[i].inst.op1.reg)) ||
				((reg1!=REG_NOP) && (reg1==cycle[i].inst.op1.reg)) ||
				((reg2!=REG_NOP) && (reg2==cycle[i].inst.op1.reg))
			)) return true;
		switch (cycle[i].inst.type)
		{
			case INSTRUCTION_TYPE_LOOP:
				if ((reg0==REG_ECX) || (reg1==REG_ECX) || (reg2==REG_ECX)) return true;
				break;
			case INSTRUCTION_TYPE_LODS:
				if ((reg0==REG_ESI) || (reg1==REG_ESI) || (reg2==REG_ESI)) return true;
				break;
			case INSTRUCTION_TYPE_STOS:
				if ((reg0==REG_EDI) || (reg1==REG_EDI) || (reg2==REG_EDI)) return true;
				break;
			default:;
		}
	}
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
	if (inst->type == INSTRUCTION_TYPE_STOS) return true;
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
			
		case INSTRUCTION_TYPE_INC:
		case INSTRUCTION_TYPE_DEC:
			
		case INSTRUCTION_TYPE_MOV:
		case INSTRUCTION_TYPE_POP:
			return true;
		default:
			return false;
	}
}