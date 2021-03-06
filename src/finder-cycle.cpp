#include "finder-cycle.h"
#include <stack>
#include <sstream>

using namespace std;

#ifdef FINDER_LOG
	#define LOG (*log)
#else
	#define LOG if (false) cerr
#endif

namespace find_decryptor
{

const uint FinderCycle::maxBackward = 20;
const uint FinderCycle::maxForward = 100;
const uint FinderCycle::maxEmulate = 180;

FinderCycle::FinderCycle(int type) : Finder(type), _in_backwards(false), am_back(0)
{
	regs_known = new bool[RegistersCount];
	regs_target = new bool[RegistersCount];
}

FinderCycle::~FinderCycle()
{
	delete[] regs_known;
	delete[] regs_target;
}
void FinderCycle::launch(int pos)
{
	LOG << " Launching from position 0x" << hex << pos << endl;
	if (start_positions.count(pos)) {
		LOG << "  Ignoring launch, already checked." << endl;
		return;
	}
	start_positions.insert(pos);
	int a[1000] = {0}, num, amount=0;
	uint barrier = 0;
	bool flag = false;
//	Command cycle[256];
	INSTRUCTION inst;
	emulator->begin(pos);
	char buff[30] = {0};
	int min_eip = emulator->get_register(EIP);
	int max_eip = 0;
	for (uint strnum = 0; strnum < maxEmulate; strnum++) {
		if (!emulator->get_command(buff)) {
			LOG << " Execution error, stopping instance." << endl;
			return;
		}
		num = emulator->get_register(EIP);
		if (!reader->is_valid(num)) {
			LOG << " Reached end of the memory block, stopping instance." << endl;
			return;
		}
		int inst_len = get_instruction(&inst, (BYTE *) buff, mode);
		if (num + inst_len > max_eip)
			max_eip = num + inst_len;
		LOG << "  Command: 0x" << hex << num << ": " << instruction_string(&inst, num) << endl;
		if (!emulator->step()) {
			LOG << " Execution error, stopping instance." << endl;
			return;
		}
		check(&inst);
		for (unsigned int i = 0; i < RegistersCount; i++) {
			if (regs_target[i] && regs_known[i]) {
				regs_target[i] = false;
			} else if (regs_target[i] && !regs_known[i]) {
				for (unsigned int k = 0; k < RegistersCount; k++) {
					regs_target[k] = regs_known[k] || regs_target[k];
					regs_known[k] = false;
				}
				bool regs_target_bak[RegistersCount], regs_known_bak[RegistersCount];
				memcpy(regs_target_bak, regs_target, RegistersCount);
				memcpy(regs_known_bak, regs_known, RegistersCount);
				_count_pop = _count_push = 0;
				_in_backwards = true;
				_push_op_target = true;
				check(&instructions_after_getpc);
				_in_backwards = false;
				int em_start = backwards_traversal(pos_getpc);
				if (em_start < 0)
				{
					memcpy(regs_target,regs_target_bak,RegistersCount);
					memcpy(regs_known,regs_known_bak,RegistersCount);
					_count_pop = _count_push = 0;
					_in_backwards = true;
					_push_op_target = false;
					check(&instructions_after_getpc);
					_in_backwards = false;
					em_start = backwards_traversal(pos_getpc);
				}
				if (em_start < 0) {
					LOG <<  " Backwards traversal failed (nothing suitable found)." << endl;
					return;
				}
				if (em_start == pos) {
					LOG <<  " Backwards traversal found the same position, this shouldn't happen!" << endl;
					return;
				}
				LOG <<  " relaunch (because of " << Registers[i] << "). New position: 0x" << hex << em_start << endl;
				return launch(em_start);
			}
		}
		if (strnum >= instructions_after_getpc.size() + am_back) {
			instructions_after_getpc.push_back(inst);
		}
		memset(regs_target,false,RegistersCount);
		int kol = 0;
		for (int i = 0; i < amount; i++) {
			if (a[i]==num) {
				kol++;
			}
		}
		if (kol >= 2) {
			int neednum = num;
			for (barrier = 0; barrier < strnum + 10; barrier++) { /// TODO: why 10?
				cycle[barrier] = Command(num,inst);
				if (!emulator->get_command(buff)) {
					LOG << " Execution error, stopping instance." << endl;
					return;
				}
				num = emulator->get_register(EIP);
				get_instruction(&inst, (BYTE *) buff, mode);
				LOG << "  Command: 0x" << hex << num << ": " << instruction_string(&inst, num) << endl;
				if (!emulator->step()) {
					LOG << " Execution error, stopping instance." << endl;
					return;
				}
				if (num==neednum) {
					flag = true;
					break;
				}
			}
		}
		if (flag) {
			break;
		}
		if (is_write_indirect(&inst)) {
			a[amount++] = num;
		}
	}
	
	if (barrier <= 0) {
		LOG << " Too short cycle, ignoring." << endl;
	} else if (flag) {
		int k = verify(cycle, barrier+1);

		if (log) {
			LOG << " Cycle found: " << endl;
			for (uint i = 0; i <= barrier; i++) {
				LOG << "  0x" << hex << cycle[i].addr << ":  " << instruction_string(&(cycle[i].inst), cycle[i].addr) << endl;
			}
			if (k != -1) {
				LOG << " Indirect write in line #" << k << ", launched from position 0x" << hex << pos << endl;
			} else {
				LOG << " No indirect writes." << endl;
			}
		}

		if (k != -1) {
			pos_dec.push_back(pos);
			dec_sizes.push_back(max_eip - min_eip);
			stringstream s;
			for (uint i = 0; i <= barrier; i++) {
				s << "  0x" << hex << cycle[i].addr << ":  " << instruction_string(&(cycle[i].inst), cycle[i].addr) << endl;
			}
			string str = s.str();
			decryptors_text.push_back(str);
			//cout << "Seeding instruction \"" << instruction_string(pos_getpc) << "\" on position 0x" << hex << pos_getpc << "." << endl;
			//cout << "Cycle found: " << endl;
			/*for (uint i = 0; i <= barrier; i++) {
				cout << " 0x" << hex << cycle[i].addr << ":  " << instruction_string(&(cycle[i].inst), cycle[i].addr) << endl;
			}*/
			//cout << " Indirect write in line #" << k << ", launched from position 0x" << hex << pos << endl;
			targets_found.insert(cycle[k-1].addr);
#ifdef FINDER_ONCE
			exit(0);
#endif
		}
	}
}

int FinderCycle::find() {
	pos_dec.clear();
	dec_sizes.clear();
	decryptors_text.clear();
	Timer::start(TimeFind);
	start_positions.clear();
	targets_found.clear();
	INSTRUCTION inst;
	uint start = reader->start(), size = reader->size();
	const unsigned char* pointer = reader->pointer();
	for (uint i=start; i<size; i++) {
		/// TODO: check opcodes
		switch (pointer[i]) {
			/// fsave/fnsave: 0x9bdd, 0xdd
			case 0x9b:
				if ((i + 1 < size) && (pointer[i+1] != 0xdd)) {
					continue;
				}
			case 0xdd:
				break;
			/// fstenv/fnstenv: 0xf2d9, 0xd9
			case 0xf2:
				if ((i + 1 < size) && (pointer[i+1] != 0xd9)) {
					continue;
				}
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
		if (!len || (len + i > size)) {
			continue;
		}
		switch (inst.type) {
			case INSTRUCTION_TYPE_FPU_CTRL:
				if (	(strcmp(inst.ptr->mnemonic,"fstenv") == 0) ||
					(strcmp(inst.ptr->mnemonic,"fsave") == 0)) {
					LOG << "Seeding instruction \"" << instruction_string(i) << "\" on position 0x" << hex << i << "." << endl;
					break;
				}
				continue;
			case INSTRUCTION_TYPE_CALL:
				if (	(strcmp(inst.ptr->mnemonic,"call") == 0) &&
					(inst.op1.type == OPERAND_TYPE_IMMEDIATE)) {
					LOG << "Seeding instruction \"" << instruction_string(i) << "\" on position 0x" << hex << i << "." << endl;
					if ((i + len + inst.op1.immediate) < size) {
						break;
					}
				}
				continue;
			default:
				continue;
		}
		pos_getpc = i;
		find_memory_and_jump(i);
		instructions_after_getpc.clear();
	}
	Timer::stop(TimeFind);
	return pos_dec.size();
}

void FinderCycle::find_memory_and_jump(int pos)
{
	INSTRUCTION inst;
	uint len;
	set<uint> nofollow;
	stack<uint> calls;

	for (uint p = pos, count_instructions = 0; p < reader->size() && count_instructions < maxForward; p += len, count_instructions++) {
		len = instruction(&inst,p);
		if (!len || (len + p > reader->size())) {
			LOG <<  " Dissasembling failed." << endl;
			return;
		}
		LOG << " Instruction: " << instruction_string(&inst,p) << " on position 0x" << hex << p << endl;
		instructions_after_getpc.push_back(inst);
		bool error = false;
		switch (inst.type) {
			/// TODO: check
			case INSTRUCTION_TYPE_JMP:
			case INSTRUCTION_TYPE_JMPC:
				if ((inst.op1.type == OPERAND_TYPE_MEMORY) && (inst.op1.basereg != REG_NOP)) {
					LOG << " Indirect jump detected: " << instruction_string(&inst) << " on position 0x" << hex << p << endl;
					get_operands(&inst);
				} else if (strcmp(inst.ptr->mnemonic,"jmp") == 0) {
					if ((inst.op1.type == OPERAND_TYPE_MEMORY) || nofollow.count(p)) {
						error = true;
					} else if (inst.op1.type == OPERAND_TYPE_IMMEDIATE) {
						nofollow.insert(p);
						p += inst.op1.immediate;
						continue;
					}
				}
				break;
			case INSTRUCTION_TYPE_CALL: /// TODO: behave like jmp?
				if (strcmp(inst.ptr->mnemonic,"call") == 0) {
					if ((inst.op1.type == OPERAND_TYPE_MEMORY) || nofollow.count(p)) {
						error = true;
					} else if (inst.op1.type == OPERAND_TYPE_IMMEDIATE) {
						nofollow.insert(p);
						calls.push(p + inst.length);
						p += inst.op1.immediate;
						continue;
					}
				}
				break;
			case INSTRUCTION_TYPE_RET: // We do not need nofollow check here, nofollow check in calls is enough.
				if (strcmp(inst.ptr->mnemonic,"ret") == 0) {
					if (calls.empty()) {
						error = true;
					} else {
						p = calls.top() - len;
						calls.pop();
						continue;
					}
				}
				break;
			default:;
		}
		if (error) {
			LOG << "   Error detected." << endl;
			return;
		}
		if (!is_write_indirect(&inst)) {
			continue;
		}
		LOG << "  Write to memory detected: " << instruction_string(&inst,p) << " on position 0x" << hex << p << endl;
		if (targets_found.count(p)) {
			LOG << "   Not running, already checked." << endl;
			return;
		}
		memset(regs_known,false,RegistersCount);
		memset(regs_target,false,RegistersCount);
		get_operands(&inst);
		_count_pop = _count_push = 0;
		_in_backwards = true;
		_push_op_target = true;
		check(&instructions_after_getpc);
		_in_backwards = false;
		int em_start = backwards_traversal(pos_getpc);
		if (em_start < 0)
		{
			memset(regs_known,false,RegistersCount);
			memset(regs_target,false,RegistersCount);
			get_operands(&inst);
			_count_pop = 0;
			_count_push = 0;
			_in_backwards = true;
			_push_op_target = false;
			check(&instructions_after_getpc);
			_in_backwards = false;
			em_start = backwards_traversal(pos_getpc);
		}
		if (em_start < 0) {
			LOG << "   Backwards traversal failed (nothing suitable found)." << endl;
			return;
		}
		print_commands(&instructions_after_getpc,1);
		launch(em_start);
		return;
	}
}

bool FinderCycle::regs_closed() {
	for (unsigned int i=0; i<RegistersCount; i++) {
		if (regs_target[i]) {
			return false;
		}
	}
	LOG << "Push-pop heuristic fails " << dec << _count_push << " " << _count_pop << endl;
	if (_count_push < _count_pop)
		return false;
	return true;
}
void FinderCycle::check(vector <INSTRUCTION>* instructions)
{
	for (vector<INSTRUCTION>::reverse_iterator rit = instructions->rbegin(); rit != instructions->rend(); rit++) {
		check(&(*rit));
	}
}
void FinderCycle::add_target(OPERAND *op) {
	if (MASK_FLAGS(op->flags) == F_f) { /// Does operand use fpu register?
		return;
	}
	int reg;
	switch (op->type) {
		case OPERAND_TYPE_REGISTER:
			reg = op->reg;
			if ((reg > 3) && (MASK_OT(op->flags) == OT_b)) { // One-byte regs. TODO: check 1or2-byte-regs
				reg -= 4; // This is a plain register
			}

			if (reg == REG_ESP) {
				break;
			}
			regs_target[int_to_reg(reg)] = true;
			break;
		case OPERAND_TYPE_MEMORY:
			if (op->basereg == REG_NOP || op->reg == REG_ESP) {
				break;
			}
			regs_target[int_to_reg(op->basereg)] = true;
			break;
		default:;
	}
}
void FinderCycle::get_operands(INSTRUCTION *inst) /// TODO: merge with check()?
{
	if (inst->type == INSTRUCTION_TYPE_LODS) {
		regs_target[ESI] = true;
	}
	if (inst->type == INSTRUCTION_TYPE_LOOP) {
		regs_target[ECX] = true;
	}
	if (inst->op1.type==OPERAND_TYPE_MEMORY) {
		add_target(&(inst->op1));
	}
	if (inst->type == INSTRUCTION_TYPE_STOS)
	{
		regs_target[EAX] = true;
		regs_target[EDI] = true;
	}
	add_target(&(inst->op2));
	add_target(&(inst->op3));
}
void FinderCycle::check(INSTRUCTION *inst)
{
	/// TODO: Add more instruction types here.
	/// TODO: WARNING: Check logic.
	int r, r2;
	bool tmp;
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
			if (inst->op1.type != OPERAND_TYPE_REGISTER) {
				break;
			}
			r = int_to_reg(inst->op1.reg);
			if ((inst->op1.type == inst->op2.type) && (inst->op1.reg == inst->op2.reg)) {
				regs_target[r] = false;
				regs_known[r] = true;
				break;
			}
			if (regs_target[r]) {
				add_target(&(inst->op2));
			}
			break;
		case INSTRUCTION_TYPE_XCHG:
			if (inst->op1.type != OPERAND_TYPE_REGISTER) {
				break;
			}
			if (inst->op2.type != OPERAND_TYPE_REGISTER) {
				break;
			}
			r = int_to_reg(inst->op1.reg);
			r2 = int_to_reg(inst->op2.reg);

			tmp = regs_known[r];
			regs_known[r] = regs_known[r2];
			regs_known[r2] = tmp;

			tmp = regs_target[r];
			regs_target[r] = regs_target[r2];
			regs_target[r2] = tmp;
			break;
		case INSTRUCTION_TYPE_ADD:
		case INSTRUCTION_TYPE_AND:
		case INSTRUCTION_TYPE_OR:
		case INSTRUCTION_TYPE_MUL:
		case INSTRUCTION_TYPE_IMUL:
			if (inst->op1.type != OPERAND_TYPE_REGISTER) {
				break;
			}
			r = int_to_reg(inst->op1.reg);
			if (regs_target[r]) {
				add_target(&(inst->op2));
			}
			break;
		case INSTRUCTION_TYPE_MOV:
		case INSTRUCTION_TYPE_LEA:
			if (inst->op1.type != OPERAND_TYPE_REGISTER) {
				break;
			}
			r = int_to_reg(inst->op1.reg);
			regs_known[r] = true;
			if (regs_target[r]) {
				regs_target[r] = false;
				add_target(&(inst->op2));
			}
			break;
		case INSTRUCTION_TYPE_POP:
			regs_target[ESP] = true;
			if (inst->op1.type == OPERAND_TYPE_NONE) {
				if (strcmp(inst->ptr->mnemonic, "popa") == 0) {
					if (_in_backwards)
						_count_pop += 8;
					regs_target[EAX] = false;
					regs_target[EBX] = false;
					regs_target[ECX] = false;
					regs_target[EDX] = false;
					regs_target[EBP] = false;
					regs_target[ESI] = false;
					regs_target[EDI] = false;
				}
				break;
			}
			if (_in_backwards)
				_count_pop++;
			if (inst->op1.type == OPERAND_TYPE_REGISTER) {
				r = int_to_reg(inst->op1.reg);
				if (r >= 0) {
					regs_known[r] = true;
					regs_target[r] = false;
				}
			}
			break;
		case INSTRUCTION_TYPE_PUSH: /// TODO: check operands
			regs_target[ESP] = false;
			if (inst->op1.type == OPERAND_TYPE_NONE) {
				if (strcmp(inst->ptr->mnemonic, "pusha") == 0) {
					if (_in_backwards)
						_count_push += 8;
					if (_push_op_target)
					{
						regs_target[EAX] = true;
						regs_target[EBX] = true;
						regs_target[ECX] = true;
						regs_target[EDX] = true;
						regs_target[EBP] = true;
						regs_target[ESI] = true;
						regs_target[EDI] = true;
					}
				}
				break;
			}
			if (_in_backwards)
				_count_push++;
			if (inst->op1.type == OPERAND_TYPE_REGISTER) {
				r = int_to_reg(inst->op1.reg);
				if (_push_op_target && r >= 0 && !regs_known[r] && Registers[r] != Registers[ESP])
				{
					regs_target[r] = true;
				}
			}
			break;
		case INSTRUCTION_TYPE_CALL:
			if (_in_backwards)
				_count_push++;
			regs_target[ESP] = false;
			regs_known[ESP] = true;
			//if (get_write_indirect(inst,&r)) {
			//	regs_target[r]=true;
			//	regs_known[r]=false;
			//}
			//if (inst->op1.type==OPERAND_TYPE_REGISTER) {
			//	r = int_to_reg(inst->op1.reg);
			//	regs_target[r]=true;
			//	regs_known[r]=false;
			//}
			break;
		case INSTRUCTION_TYPE_OTHER:
			if (strcmp(inst->ptr->mnemonic,"cpuid")==0) {
				regs_target[EAX] = true;
				regs_target[EBX] = false;
				regs_target[ECX] = false;
				regs_target[EDX] = false;
				regs_known[EBX] = true;
				regs_known[ECX] = true;
				regs_known[EDX] = true;
			}
			break;
		case INSTRUCTION_TYPE_FPU_CTRL:
			_count_push += 12;
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

int FinderCycle::backwards_traversal(int pos)
{
	am_back = 0;
	if (regs_closed()) {
		return pos;
	}
	_in_backwards = true;
	bool regs_target_bak[RegistersCount], regs_known_bak[RegistersCount];
	memcpy(regs_target_bak,regs_target,RegistersCount);
	memcpy(regs_known_bak,regs_known,RegistersCount);
	int _count_pop_bak = _count_pop;
	int _count_push_bak = _count_push;
	vector <unsigned int> queue[2];
	map<int,INSTRUCTION> instructions;
	INSTRUCTION inst;
	queue[0].push_back(pos);
	int m = 0;
	vector <INSTRUCTION> commands;
	for (uint n = 0; n < maxBackward; n++) {
		queue[m^1].clear();
		for (vector<unsigned int>::iterator p=queue[m].begin(); p!=queue[m].end(); p++) {
			for (unsigned int i=1; (i<=MaxCommandSize) && (i<=(*p)); i++) {
				int curr = (*p) - i;
				bool ok = false;
				unsigned int len = instruction(&inst,curr);
				switch (inst.type) {
					case INSTRUCTION_TYPE_JMP:
					case INSTRUCTION_TYPE_JMPC:
					case INSTRUCTION_TYPE_JECXZ:
						ok = (i == (inst.op1.immediate + len));
						break;
					default:;
				}
				if (len!=i && !ok) {
					continue;
				}
				instructions[curr] = inst;
				queue[m^1].push_back((*p)-i);
				commands.clear();
				for (uint j=curr,k=0; k<=n; k++) {
					commands.push_back(instructions[j]);
					j += instructions[j].length;
				}
				check(&commands);
				am_back = commands.size();
				bool ret = regs_closed();
/*				if (log) {
					LOG << "   BACKWARDS TRAVERSAL ITERATION" << endl;
					print_commands(&commands, 0);
					if (ret) {
						LOG << "Backwards traversal iteration succeeded." << endl;
					} else {
						LOG << "Backwards traversal iteration failed. Unknown registers: ";
						for (int i=0; i<RegistersCount; i++) {
							if (regs_target[i]) {
								LOG << " " << dec << i;
							}
						}
						LOG << endl;
					}
				}*/
				if (ret) {
					_in_backwards = false;
					return curr;
				}
				memcpy(regs_target,regs_target_bak,RegistersCount);
				memcpy(regs_known,regs_known_bak,RegistersCount);
				_count_pop = _count_pop_bak;
				_count_push = _count_push_bak;
			}
		}
		m ^= 1;
		/// TODO: We should also check all static jumps to this point.
	}
	_in_backwards = false;
	return -1;
}
int FinderCycle::verify(Command *cycle, int size)
{
	for (int i=0;i<size;i++) {
		if (is_write_indirect(&(cycle[i].inst))) {
			if (verify_changing_reg(&(cycle[i].inst), cycle, size)) {
				return i+1;
			}
		}
	}
	return -1;
}

bool FinderCycle::verify_changing_reg(INSTRUCTION *inst, Command *cycle, int size)
{
	int	mem  = inst->op1.displacement,
		reg0 = inst->op1.basereg,
		reg1 = inst->op1.reg,
		reg2 = inst->op1.indexreg;
	if (reg0 != REG_NOP) {
		mem += emulator->get_register((Register) int_to_reg(reg0));
	}
	if (reg1 != REG_NOP) {
		mem += emulator->get_register((Register) int_to_reg(reg1));
	}
	if (reg2 != REG_NOP) {
		mem += emulator->get_register((Register) int_to_reg(reg2));
	}
	if (inst->type == INSTRUCTION_TYPE_STOS) {
		reg0 = REG_EDI;
		mem = emulator->get_register((Register) int_to_reg(reg0));
	}
	mem -= emulator->memory_offset();
	if ((mem==0) || !reader->is_within_one_block(mem,cycle[0].addr)) {
		return false;
	}
	for (int i=0;i<size;i++) {
		if (	is_write(&(cycle[i].inst)) && 
			(cycle[i].inst.op1.type == OPERAND_TYPE_REGISTER) && 
			(	((reg0 != REG_NOP) && (reg0 == cycle[i].inst.op1.reg)) ||
				((reg1 != REG_NOP) && (reg1 == cycle[i].inst.op1.reg)) ||
				((reg2 != REG_NOP) && (reg2 == cycle[i].inst.op1.reg))
			)) {
			return true;
		}
		switch (cycle[i].inst.type) {
			case INSTRUCTION_TYPE_LOOP:
				if ((reg0 == REG_ECX) || (reg1 == REG_ECX) || (reg2 == REG_ECX)) {
					return true;
				}
				break;
			case INSTRUCTION_TYPE_LODS:
				if ((reg0 == REG_ESI) || (reg1 == REG_ESI) || (reg2 == REG_ESI)) {
					return true;
				}
				break;
			case INSTRUCTION_TYPE_STOS:
				if ((reg0 == REG_EDI) || (reg1 == REG_EDI) || (reg2 == REG_EDI)) {
					return true;
				}
				break;
			default:;
		}
	}
	return false;
}
void FinderCycle::dump_regs() {
#ifdef FINDER_LOG
	LOG << "   Regs target:";
	for (int rx = 0; rx < RegistersCount; rx++) {
		if (regs_target[rx]) {
			LOG << " " << Registers[rx];
		}
	}
	LOG << endl;
	LOG << "   Regs known:";
	for (int rx = 0; rx < RegistersCount; rx++) {
		if (regs_known[rx]) {
			LOG << " " << Registers[rx];
		}
	}
	LOG << endl;
#endif
}

} //namespace find_decryptor