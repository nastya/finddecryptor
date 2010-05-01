#include "finder.h"

using namespace std;

const Mode Finder::mode = MODE_32;
const Format Finder::format = FORMAT_INTEL;

Command::Command(int a, INSTRUCTION i, string s) {
	addr = a;
	inst = i;
	str = s;
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
	log->close();
	delete log;
}
void Finder::launch(int start, int pos)
{
	(*log) << "launch (0x" << hex << start << ", 0x" << hex << pos << ")" << endl;
	int a[1000]={0}, i,k,num,amount=0, barrier;
	bool flag = false;
	Command cycle[256];
	string command;
	INSTRUCTION inst;
	emulator.begin(start,pos);
	char buff[10] = {0};
	for (int strnum=0;;strnum++)
	{
		if (!emulator.get_command(buff)) { /// Die on error, it will get SIGSEGV anyway.
			(*log) << " Execution error, stopping instance." << endl;
			return;
		}
		num = emulator.get_register(EIP);
		get_instruction(&inst, (BYTE *) buff, mode);
		command = instruction_string(&inst, num);
		emulator.step();
		(*log) << "  Command: 0x" << hex << num << ": " << command << endl;
		get_operands(command);
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
					(*log) <<  " Backwards traversal failed (nothing suitable found)." << endl;
					return;
				}
				int startn = reader.get_starting_point(), posn = reader.calculate_virt_addr(em_start);
				(*log) <<  " relaunch (because of " << Registers[i] << ") 0x" << hex << startn << " 0x" << hex << posn << endl;
//				if ((start==startn)&&(pos==posn)) return; /// TODO: check this
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
				cycle[barrier] = Command(num,inst,command);
				if (!emulator.get_command(buff)) { /// Die on error, it will get SIGSEGV anyway.
					(*log) << " Execution error, stopping instance." << endl;
					return;
				}
				num = emulator.get_register(EIP);
				get_instruction(&inst, (BYTE *) buff, mode);
				command = instruction_string(&inst, num);
				emulator.step();
				(*log) << "  Command: 0x" << hex << num << ": " << command << endl;
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
		
		if (k!=-1)
		{
			cout << "Cycle found: " << endl;
			for (i=0; i<=barrier; i++)
			{
				cout << " 0x" << hex << cycle[i].addr << ":  " << cycle[i].str << endl;
			}
			cout << "Indirect write in line #" << k << ", launched: (0x" << hex << start << ", 0x" << hex << pos << ")" << endl;
		}
		
		(*log) << " Cycle found: " << endl;
		for (i=0; i<=barrier; i++)
		{
			(*log) << " 0x" << hex << cycle[i].addr << ":  " << cycle[i].str << endl;
		}
		if (k!=-1)
		{
			(*log) << " Indirect write in line #" << k << ", launched: (0x" << hex << start << ", 0x" << hex << pos << ")" << endl;
		} else {
			(*log) << " No indirect writes." << endl;
		}
	}
}

void Finder::init()
{
	regs_known = new bool[RegistersCount];
	regs_target = new bool[RegistersCount];
	data = NULL;
	dataSize = 0;
	log = new ofstream("../log.txt");
}
void Finder::clear()
{
	delete[] data;
	data = NULL;
	dataSize = 0;
}
string Finder::instruction_string(INSTRUCTION *inst, int pos) {
	char str[256];
	get_instruction_string(inst, format, (DWORD)pos, str, sizeof(str));
	return (string) str;
}
string Finder::instruction_string(BYTE *data, int pos) {
	INSTRUCTION inst;
	get_instruction(&inst, data, mode);
	return instruction_string(&inst,pos);
}
string Finder::instruction_string(int pos) {
	INSTRUCTION inst;
	get_instruction(&inst, &data[pos], mode);
	return instruction_string(&inst,pos);
}
void Finder::find() 
{
	INSTRUCTION inst;
	for (int i = 0; i < dataSize; i++)
	{
		int len = get_instruction(&inst, &data[i], mode);		
		if (!len || (len + i > dataSize)) continue;
		if (strcmp(inst.ptr->mnemonic,"call")==0||strcmp(inst.ptr->mnemonic,"fstenv")==0||strcmp(inst.ptr->mnemonic,"fnstenv")==0)
		{
			(*log) << "Instruction \"" << instruction_string(i) << "\" on position " << dec << i << "." << endl;
			pos_getpc=i;
			find_memory(i);
			find_jump(i);
			(*log) << "*********************************************************" << endl;
		}
		instructions_after_getpc.clear();	
	}
}

void Finder::find_memory(int pos)
{
	INSTRUCTION inst;
	int len;
	for (int p=pos; p<dataSize; p+=len)
	{
		len = get_instruction(&inst,&data[p],mode);
		if (!len || (len + p > dataSize))
		{
			p++;
			continue;
		}
		instructions_after_getpc.push_back(inst);
		if (inst.op1.type != OPERAND_TYPE_MEMORY) continue;
		/// TODO: add some int-based logic here instead of string-based.
		string str = instruction_string(&inst,p);
		string::size_type popen = str.find('['), pclose = str.find(']'), pcomma = str.find(',');
		if ((popen==string::npos) || (pclose<popen) || (pcomma<pclose)) continue;
		for (int i=0; i<CommandsChangingCount; i++)
			if (strcmp(inst.ptr->mnemonic,CommandsChanging[i])==0)
			{
				for (int j=0;j<RegistersCount;j++)
				{
					string::size_type preg = str.find(Registers[j]);
					if ((preg==string::npos) || (preg<popen) || (pclose<preg)) continue;
					(*log) << "Write to memory detected: " << str << " on position " << p << endl;
					if (start_positions.count(p))
					{
						(*log) << "Not running, already checked." << endl;
						return;
					}
					start_positions.insert(p);
					memset(regs_known,false,RegistersCount);
					memset(regs_target,false,RegistersCount);
					get_operands(p);
					check(&instructions_after_getpc);
					int em_start = backwards_traversal(pos);
					if (em_start<0)
					{
						(*log) <<  " Backwards traversal failed (nothing suitable found)." << endl;
						return;
					}
					print_commands(&instructions_after_getpc,1);
					launch(reader.get_starting_point(),reader.calculate_virt_addr(em_start));
					return;
				}
			}
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
		check((*instructions)[k]);
}
void Finder::check(INSTRUCTION inst)
{
	string str = instruction_string(&inst);
	string::size_type preg, pcomma;
	/// TODO: add more instruction types here
	switch (inst.type)
	{
		case INSTRUCTION_TYPE_XOR:
		case INSTRUCTION_TYPE_SUB:
		case INSTRUCTION_TYPE_SBB:
		case INSTRUCTION_TYPE_DIV:
		case INSTRUCTION_TYPE_IDIV:
			pcomma = str.find(',');
			for (int i=0; i<RegistersCount; i++)
			{
				if (!regs_target[i]) continue;
				preg = str.find(Registers[i]);
				if ((preg==string::npos) || (pcomma<preg) || (str[preg-1]!=' ')) continue;
				for (int j=0; j<RegistersCount; j++)
				{
					if (str.find(Registers[j],pcomma)==string::npos) continue;
					regs_target[j] = (i!=j);
					break;
				}
			}
			for (int i=0; i<RegistersCount; i++)
			{
				preg = str.find(Registers[i]);
				if ((preg==string::npos) || (str[preg-1]!=' ') || (pcomma<preg) || (str.find(Registers[i],pcomma)==string::npos)) continue;
				regs_known[i] = true;
				break;
			}
			break;
		case INSTRUCTION_TYPE_ADD:
		case INSTRUCTION_TYPE_AND:
		case INSTRUCTION_TYPE_OR:
		case INSTRUCTION_TYPE_MUL:
		case INSTRUCTION_TYPE_IMUL:
			pcomma = str.find(',');
			for (int i=0; i<RegistersCount; i++)
			{
				if (!regs_target[i]) continue;
				preg = str.find(Registers[i]);
				if ((preg==string::npos) || (pcomma<preg) || (str[preg-1]!=' ')) continue;
				for (int j=0; j<RegistersCount; j++)
				{
					if (str.find(Registers[j],pcomma)==string::npos) continue;
					regs_target[j] = true;
					break;
				}
			}
			break;
		case INSTRUCTION_TYPE_MOV:
			pcomma = str.find(',');
			for (int i=0; i<RegistersCount; i++)
			{
				if (!regs_target[i]) continue;
				preg = str.find(Registers[i]);
				if ((preg==string::npos) || (str[preg-1]!=' ') || (pcomma<preg)) continue;
				regs_target[i] = false;
				break;
			}
			for (int i=0; i<RegistersCount; i++)
			{
				preg = str.find(Registers[i]);
				if ((preg==string::npos) || (str[preg-1]!=' ') || (pcomma<preg)) continue;
				regs_known[i] = true;
				break;
			}
			break;
		case INSTRUCTION_TYPE_POP:
			for (int i=0; i<RegistersCount; i++)
			{
				if (!regs_target[i]) continue;
				preg = str.find(Registers[i]);
				if ((preg==string::npos) || (str[preg-1]!=' ')) continue;
				regs_target[i] = false;
				regs_target[ESP] = true;
				break;
			}
			for (int i=0; i<RegistersCount; i++)
			{
				preg = str.find(Registers[i]);
				if ((preg==string::npos) || (str[preg-1]!=' ')) continue;
				regs_known[i] = true;
				break;
			}
			break;
		case INSTRUCTION_TYPE_FPU:	/// TODO: add more filters for this one
			if (strcmp(inst.ptr->mnemonic,"fldz")==0) break;
		case INSTRUCTION_TYPE_FCMOVC:
			regs_target[ESP] = false;
			regs_known[ESP] = true;
			break;
	}
	smaller_to_greater_regs();
}

void Finder::print_commands(vector <INSTRUCTION>* v, int start)
{
	char str[256];
	int i=0;
	vector<INSTRUCTION>::iterator p;
	(*log) << " Commands in queue:"<< endl;
	for (vector<INSTRUCTION>::iterator p=v->begin(); p!=v->end(); i++,p++)
	{
		if (i<start) continue;
		(*log) << "  " << instruction_string(&(*p)) << endl;
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

int Finder::verify(Command cycle[256],int size)
{
	int i,reg;
	for (i=0;i<size;i++)
	{
		if (is_indirect_write(cycle[i].str, &reg))
		{
			if (verify_changing_reg(cycle, size, reg))
				return i+1;
		}
	}
	return -1;
}

bool Finder::verify_changing_reg(Command cycle[256], int size, int reg)
{
	if (!reader.is_within_one_block(emulator.get_register((Register)reg),cycle[0].addr))
		return false;
	for (int i=0;i<size;i++)
	{
		for (int j=0;j<CommandsChangingCount;j++)
		{
			if (cycle[i].str.find(CommandsChanging[j])==string::npos) continue; 
			int p = cycle[i].str.find(Registers[reg]);
			if ((p!=string::npos)&&(cycle[i].str[p-1]==' ')) /// op reg...
				return true;
		}
	}
	return false;
}
bool Finder::is_indirect_write(string str, int *reg)
{
	for (int i=0;i<CommandsChangingCount;i++)
	{
		if (str.find(CommandsChanging[i])==string::npos) continue; 
		for (int j=0;j<RegistersCount;j++)
		{
			(*reg) = j;
			int p = str.rfind(Registers[j]);
			if ((p!=string::npos)&&(str[p-1]=='[')&&(str[p-2]==' ')) /// op [reg...],
				return true;
		}
	}
	return false;
}

void Finder::get_operands(int pos)
{
	return get_operands(instruction_string(pos));
}
void Finder::get_operands(string str)
{
	if (str.find("loop")!=string::npos) 
	{
		regs_target[ECX] = true;
	}
	for (int i=0; i<RegistersCount; i++)
	{
		if (regs_target[i]) continue;
		int p = str.find(Registers[i]);
		if (p==string::npos) continue;
		switch (str[p-1]) {
			case '[': case ',': case '+': case '-':
				regs_target[i] = true;
				/// TODO: split large registers into smaller ones?
				break;
		}
	}
	smaller_to_greater_regs();
}
void Finder::smaller_to_greater_regs()
{
	for (int i=AL;i<DL;i++)
		if (regs_target[i])
		{
			regs_target[EAX-AL+i] = true;
			regs_target[i] = false;
		}
	for (int i=AH;i<DH;i++)
		if (regs_target[i])
		{
			regs_target[EAX-AH+i] = true;
			regs_target[i] = false;
		}
	for (int i=AX;i<DX;i++)
		if (regs_target[i])
		{
			regs_target[EAX-AX+i] = true;
			regs_target[i] = false;
		}
}

int Finder::find_jump(int pos)
{
	INSTRUCTION inst;
	int len;
	for (; pos<dataSize; pos+=len)
	{
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
				string str = instruction_string(&inst);
				string::size_type pspace = str.find(' ');
				if (pspace==string::npos) continue;
				for (int i=0; i<RegistersCount; i++)
				{
					string::size_type preg = str.find(Registers[i]);
					if ((preg==string::npos) || (preg<pspace)) continue;
					(*log) << " Indirect jump detected: " << str << " on position " << dec << pos << endl;
					get_operands(pos);
					return pos;
				}
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
		(*log) << "Error opening file!\n";
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