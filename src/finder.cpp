#include "finder.h"

using namespace std;

const int Finder::MaxCommandSize = 10;
const Mode Finder::mode = MODE_32;
const Format Finder::format = FORMAT_INTEL;
const char* Finder::Registers [] = {"eax","edx","ecx","ebx","edi","esi","ebp","esp","ax","dx","cx","bx","di","si","bp","sp",
"ah","dh","ch","bh","al","dl","cl","bl" }; 
const char* Finder::CommandsChanging [] = {"xor","add","and","or","sub","mul","imul","div","mov","pop"};
const int Finder::RegistersCount = sizeof(Registers)/sizeof(Registers[0]);
const int Finder::CommandsChangingCount = sizeof(CommandsChanging)/sizeof(CommandsChanging[0]);

Finder::Finder()
{
	init();
}

Finder::Finder(char* name)
{
	init();
	read_file(name);
}

Finder::~Finder()
{
	clear();
	delete[] regs_target;
}

void Finder::init()
{
	regs_target = new bool[RegistersCount];
	memset(regs_target,false,RegistersCount);
	data = NULL;
	dataSize = 0;
}
void Finder::clear()
{
	memset(regs_target,false,RegistersCount);
	delete[] data;
	data = NULL;
	dataSize = 0;
}

void Finder::find() {
	INSTRUCTION inst;
	for (int i = 0; i < dataSize; i++)
	{
		int len = get_instruction(&inst, &data[i], mode);
		if (!len || (len + i > dataSize)) continue;
		if (strcmp(inst.ptr->mnemonic,"call")==0||strcmp(inst.ptr->mnemonic,"fstenv")==0||strcmp(inst.ptr->mnemonic,"fnstenv")==0)
		{
			char string1[256]; 
			get_instruction_string(&inst, format, (DWORD)i, string1, sizeof(string1));
			cout << "Instruction \"" << string1 << "\" on position " << i << "." << endl;
			find_memory(i);
			find_jump(i);
			cout << "*********************************************************" << endl;
		}
	}
}

int Finder::find_memory(int pos)
{
	INSTRUCTION inst;
	int len;
	vector <INSTRUCTION> instructions;
	for (int p=pos; p<dataSize; p+=len)
	{
		len = get_instruction(&inst,&data[p],mode);
		if (!len || (len + p > dataSize))
		{
			p++;
			continue;
		}
		instructions.push_back(inst);
		if (inst.op1.type != OPERAND_TYPE_MEMORY) continue;
		/// TODO: add some inst-based logic here instead of string-based.
		char string1[256];
		get_instruction_string(&inst, format, (DWORD)p, string1, sizeof(string1));
		char *popen = index(string1,'['), *pclose = index(string1,']'), *pcomma = index(string1,',');
		if ((popen==NULL)||(pclose<popen)||(pcomma<pclose)) continue;
		for (int i=0; i<CommandsChangingCount; i++)
			if (strcmp(inst.ptr->mnemonic,CommandsChanging[i])==0)
			{
				for (int j=0;j<RegistersCount;j++)
				{
					char *preg = strstr(string1,Registers[j]);
					if ((preg<popen)||(pclose<preg)) continue;
					cout << "Write to memory detected: " << string1 << " on position " << p << endl;
					get_operands(p);
					check1(&instructions); // Checking whether operands of target instruction are defined
					backwards_traversal(pos);
					print_commands(&instructions,1);
					memset(regs_target,false,RegistersCount);
					return p;
				}
			}
	}
	return -1;
}

void Finder::check1(vector <INSTRUCTION>* instructions)
{
	for (int k=instructions->size()-1;k>=0;k--)
	{
		check_inst((*instructions)[k]);
	}
}

bool Finder::check2(vector <int>* queue, vector <int>* prev)
{
	vector <INSTRUCTION> commands;
	get_commands(&commands,queue,prev); 
	check1(&commands);
	for (int i=0; i<RegistersCount; i++) {
		if (regs_target[i]) return false;
	}
	return true;
}

void Finder::get_commands(vector <INSTRUCTION>* commands, vector <int>* num_commands, vector <int>* prev)
{
	INSTRUCTION inst;
	for (int i=num_commands->size()-1;i!=-1;i=(*prev)[i])
	{
		get_instruction(&inst,&data[(*num_commands)[i]],mode);
		commands->push_back(inst);
	}
}

void Finder::check_inst(INSTRUCTION inst)
{
	char string1[256];
	get_instruction_string(&inst, format, 0, string1, sizeof(string1));
	char *pos_reg, *pos_comma;
	/// TODO: add more instruction types here
	switch (inst.type)
	{
		case INSTRUCTION_TYPE_XOR:
		case INSTRUCTION_TYPE_SUB:
		case INSTRUCTION_TYPE_SBB:
		case INSTRUCTION_TYPE_DIV:
		case INSTRUCTION_TYPE_IDIV:
			pos_comma = index(string1,',');
			for (int i=0; i<RegistersCount; i++)
			{
				if (!regs_target[i]) continue;
				pos_reg = strstr(string1,Registers[i]);
				if ((pos_reg!=NULL)&&(pos_comma>pos_reg)&&(*(pos_reg-1)==' '))
				{
					if (strstr(pos_comma,Registers[i])!=NULL)
					{
						regs_target[i] = false;
					}
					for (int t=0; t<RegistersCount; t++)
					{
						if (strstr(pos_comma,Registers[t])!=NULL)
						{
							regs_target[t] = true;
							break;
						}
					}
				}
			}
			break;
		case INSTRUCTION_TYPE_ADD:
		case INSTRUCTION_TYPE_AND:
		case INSTRUCTION_TYPE_OR:
		case INSTRUCTION_TYPE_MUL:
		case INSTRUCTION_TYPE_IMUL:
			pos_comma = index(string1,',');
			for (int i=0; i<RegistersCount; i++)
			{
				if (!regs_target[i]) continue;
				pos_reg = strstr(string1,Registers[i]);
				if ((pos_reg!=NULL)&&(pos_comma>pos_reg)&&(*(pos_reg-1)==' '))
				{
					for (int t=0; t<RegistersCount; t++)
					{
						if (strstr(pos_comma,Registers[t])!=NULL)
						{
							regs_target[t] = true;
							break;
						}
					}
				}
			}
			break;
		case INSTRUCTION_TYPE_MOV:
			pos_comma = index(string1,',');
			for (int i=0; i<RegistersCount; i++)
			{
				if (!regs_target[i]) continue;
				pos_reg = strstr(string1,Registers[i]);
				if ((pos_reg!=NULL)&&(pos_comma>pos_reg)&&(*(pos_reg-1)==' '))
				{
					regs_target[i] = false;
					break;
				}
			}
			break;
		case INSTRUCTION_TYPE_POP:
			for (int i=0; i<RegistersCount; i++)
			{
				if (!regs_target[i]) continue;
				pos_reg = strstr(string1,Registers[i]);
				if ((pos_reg!=NULL)&&(*(pos_reg-1)==' '))
				{
					regs_target[i] = false;
					regs_target[ESP] = true;
					break;
				}
			}
			break;
		case INSTRUCTION_TYPE_FCMOVC:
			regs_target[ESP] = false;
			break;
	}
}

void Finder::print_commands(vector <INSTRUCTION>* v, int start)
{
	char string1[256];
	int i=0;
	vector<INSTRUCTION>::iterator p;
	for (vector<INSTRUCTION>::iterator p=v->begin(); p!=v->end(); i++,p++)
	{
		if (i<start) continue;
		get_instruction_string(&(*p), format, 0, string1, sizeof(string1));
		cout << string1 << endl;
	}
}

void Finder::backwards_traversal(int pos)
{
	INSTRUCTION inst;
	char string1[256];
	int length=1;
	bool regs_target_bak[RegistersCount];
	memcpy(regs_target_bak,regs_target,RegistersCount);
	vector <int> queue, prev;
	queue.push_back(pos);
	prev.push_back(-1);
	for (int cur=0; cur<length;)
	{
		for (int i=1;i<=MaxCommandSize;i++)
		{
			if (get_instruction(&inst,&data[queue[cur]-i],mode)==i)
			{
				queue.push_back(queue[cur]-i);
				prev.push_back(cur);
				length++;
				if (check2(&queue,&prev))
				{
					length=0;
					break;
				}
				else
				{
					memcpy(regs_target,regs_target_bak,RegistersCount);
				}
			}
		}
		cur++;
	}
	vector <INSTRUCTION> commands;
	get_commands(&commands,&queue,&prev); 
	print_commands(&commands);
}

void Finder::get_operands(int pos)
{
	INSTRUCTION inst;
	char string1[256], *p;
	get_instruction(&inst,&data[pos],mode);
	get_instruction_string(&inst,format,(DWORD)pos,string1,sizeof(string1));
	for (int i=0; i<RegistersCount; i++)
	{
		if (regs_target[i]) continue;
		p = strstr(string1,Registers[i]);
		if (p==NULL) continue;
		switch (*(p-1)) {
			case '[': case ' ': case ',': case '+': case '-':
				regs_target[i] = true;
				/// TODO: split large registers into smaller ones?
				break;
		}
	}
}

int Finder::find_jump(int pos)
{
	INSTRUCTION inst;
	char string1[256]; 
	int len;
	for (; pos<dataSize; pos+=len)
	{
		len = get_instruction(&inst,&data[pos],mode);
		if (!len || (len + pos > dataSize))
		{
			pos++;
			continue;
		}
		get_instruction_string(&inst, format, (DWORD)pos, string1, sizeof(string1));
		if (string1[0]=='j')
		{
			char *pspace = index(string1,' ');
			if (pspace==NULL) continue;
			for (int i=0; i<RegistersCount; i++)
			{
				if (strstr(string1,Registers[i]) > pspace)
				{
					cout << "Indirect jump detected: " << string1 << " on position " << pos << endl;
					get_operands(pos);
					return pos;
				}
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
		cout << "Error opening file\n";
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