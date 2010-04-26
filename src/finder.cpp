#include "finder.h"

using namespace std;

const Mode Finder::mode = MODE_32;
const Format Finder::format = FORMAT_INTEL;

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
	delete[] regs_target;
	delete[] all_regs_target;
	out->close();
	log->close();
	delete out;
}
void Finder::launch(int start, int pos)
{
	(*log) << "launch (0x" << hex << start << ", 0x" << hex << pos << ")" << endl;
	char str[1024];
	char auxstr[10];
	int a[1000],a_amount[1000];
	int i,k,num,amount=0,neednum, barrier,kol,em_start;
	bool flag;
	char cycle[256][1024];
	char string1[256];
	emulator.begin(start,pos);
	int val;
	for(int strnum=0;;strnum++)
	{
		strcpy(str,emulator.get_command_string().c_str());
		num=get_address(str);
		get_operands_string(str);
		for(i=0;i<RegistersCount;i++)
		{
			if (regs_target[i]&&all_regs_target[i])
			{
				regs_target[i]=false;
			}
			else if (regs_target[i]&&!all_regs_target[i])
			{
				for(k=0;k<RegistersCount;k++)
				{
					if (!regs_target[k])
					{
						regs_target[k]=all_regs_target[k];
					} else {
						all_regs_target[k]=regs_target[k];
					}
				}
				check1(&instructions_after_getpc); // Checking whether operands of target instruction are defined
				em_start=backwards_traversal(pos_getpc);
				emulator.end();				
				int startn = reader.get_starting_point(), posn = reader.calculate_virt_addr(em_start);
				(*log)<< " relaunch " << dec << em_start <<" 0x" << hex << startn << " 0x" << hex << posn << endl;
//				if ((start==startn)&&(pos==posn)) return; /// TODO: check this
				return launch(startn,posn);
			}
		}
		memset(regs_target,false,RegistersCount);
		flag=false;
		kol=0;
		for(i=0;i<amount;i++)
		{
			if (a[i]==num)
			{
				kol++;
			}
		}
		if (kol>=2)
		{
			neednum=num;
			for(barrier=0;barrier<strnum+10;barrier++)
			{
				for(k=0;k<1024;k++) cycle[barrier][k]=str[k];
				strcpy(str,emulator.get_command_string().c_str());
				num=get_address(str);
				if (num==neednum)
				{
					flag=true;
					break;
				}
			}
		}
		if (flag) break;
		a[amount++]=num;
	}
	
	
	if (!flag) return;
	
	(*log)<<" Cycle found: "<<endl;
	for(i=0;i<=barrier;i++)
	{
		(*log)<<"  "<<cycle[i]<<endl;
	}
	if ((k=verify(cycle,barrier+1))!=-1)
	{
		(*log)<<" Indirect write in "<< k << " line, launched: (0x" << hex << start << ", 0x" << hex << pos << ")" << endl;
		
		(*out)<<"Cycle found: "<<endl;
		for(i=0;i<=barrier;i++)
		{
			(*out)<<" "<<cycle[i]<<endl;
		}
		(*out)<<"Indirect write in "<< k << " line, launched: (0x" << hex << start << ", 0x" << hex << pos << ")" << endl;
	} else {
		(*log)<<" No indirect writes." << endl;
	}
	emulator.end();
}

void Finder::init()
{
	regs_target = new bool[RegistersCount];
	all_regs_target = new bool[RegistersCount];
	memset(regs_target,false,RegistersCount);
	memset(all_regs_target,false,RegistersCount);
	data = NULL;
	dataSize = 0;
	out = new ofstream("../output.txt");
	log = new ofstream("../log.txt");
}
int Finder::get_address(char *str)
{
	char auxstr[10];
	int i,k;
	for(i=0;str[i]!='x';i++);
	for(i++,k=0;str[i]!=':';i++,k++)
	{
		auxstr[k]=str[i];
		
	}
	auxstr[k]='\0';
	return str_to_int(auxstr);
}

int Finder::str_to_int(const char *str)
{
	int num;
	sscanf(str,"%X",&num); ///TODO: is it really %X we need to use? May be %x?
	return num;
}
void Finder::int_to_str(int num, char *s)
{
	sprintf(s,"%x",num);
}
int Finder::get_address_jump(char *str)
{
	char auxstr[10];
	int i,k;
	for(i=0;str[i]!='x';i++);
	for(i++;str[i]!='x';i++);
	for(i++,k=0;str[i]!='\0';i++,k++)
	{
		auxstr[k]=str[i];
		
	}
	auxstr[k]='\0';
	return str_to_int(auxstr);
}
void Finder::clear()
{
	memset(regs_target,false,RegistersCount);
	memset(all_regs_target,false,RegistersCount);
	delete[] data;
	data = NULL;
	dataSize = 0;
}
string Finder::instruction_string(int pos) {
	char string1[256]; 
	INSTRUCTION inst;
	get_instruction(&inst, &data[pos], mode);
	get_instruction_string(&inst, format, (DWORD)pos, string1, sizeof(string1));
	return (string) string1;
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

int Finder::find_memory(int pos)
{
	INSTRUCTION inst;
	int len,em_start;
	//vector <INSTRUCTION> instructions;
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
					(*log) << "Write to memory detected: " << string1 << " on position " << p << endl;
					get_operands(p);
					check1(&instructions_after_getpc); // Checking whether operands of target instruction are defined
					em_start=backwards_traversal(pos);
					print_commands(&instructions_after_getpc,1);
					launch(reader.get_starting_point(),reader.calculate_virt_addr(em_start));
					memset(regs_target,false,RegistersCount);
					memset(all_regs_target,false,RegistersCount);
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
						break;
					}
					for (int t=0; t<RegistersCount; t++)
					{
						if (strstr(pos_comma,Registers[t])!=NULL)
						{
							regs_target[t] = true;
							all_regs_target[t] = true;
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
							all_regs_target[t] = true;
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
					all_regs_target[ESP] = true;
					break;
				}
			}
			break;
//		case INSTRUCTION_TYPE_FPU:	/// TODO: add more filters for this one
		case INSTRUCTION_TYPE_FCMOVC:
			regs_target[ESP] = false;
			break;
	}
	smaller_to_greater_regs();
}

void Finder::print_commands(vector <INSTRUCTION>* v, int start)
{
	char string1[256];
	int i=0;
	vector<INSTRUCTION>::iterator p;
	(*log) << " Commands in queue :"<< endl;
	for (vector<INSTRUCTION>::iterator p=v->begin(); p!=v->end(); i++,p++)
	{
		if (i<start) continue;
		get_instruction_string(&(*p), format, 0, string1, sizeof(string1));
		(*log) << "  " << string1 << endl;
	}
}

int Finder::backwards_traversal(int pos)
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
	return queue[queue.size()-1];
	
}

int Finder::verify(char cycle[256][1024],int size)
{
	int i,reg;
	for(i=0;i<size;i++)
	{
		if (is_indirect_write(cycle[i], &reg))
		{
			if (verify_changing_reg(cycle, size, reg))
				return i+1;
		}
	}
	return -1;
}

bool Finder::verify_changing_reg(char cycle[256][1024], int size, int reg)
{
	char *str, *p;
	if (!reader.is_within_one_block(emulator.get_register((Register)reg),get_address(cycle[0])))
		return false;
	for(int i=0;i<size;i++)
	{
		str=cycle[i];
		for(int j=0;j<CommandsChangingCount;j++)
		{
			if (strstr(str,CommandsChanging[j])!=NULL)
			{
				p=strstr(str,Registers[reg]);
				if ((p!=NULL) && ((*(p-1))=='%') && ((*(p-2))==','))
					return true;
			}
		}
	}
	return false;
}
bool Finder::is_indirect_write(char * str, int * reg)
{
	char *p;
	int i,j;
	for(i=0;i<CommandsChangingCount;i++)
	{
		if (strstr(str,CommandsChanging[i])!=NULL)
		{
			for(j=0;j<RegistersCount;j++)
			{
				(*reg)=j;
				p=strstr(str,Registers[j]);
				if ((p!=NULL) && (p>strchr(str,',')) && ((*(p-1))=='%') && ((*(p-2))=='('))
					return true;
			}
		}
	}
	return false;
}

void Finder::get_operands(int pos)
{
	INSTRUCTION inst;
	char string1[256], *p;
	get_instruction(&inst,&data[pos],mode);
	get_instruction_string(&inst,format,(DWORD)pos,string1,sizeof(string1));
	if (strstr(string1,"loop")!=NULL) 
	{
		regs_target[ECX]=true;
		all_regs_target[ECX]=true;
	}
	for (int i=0; i<RegistersCount; i++)
	{
		if (regs_target[i]) continue;
		p = strstr(string1,Registers[i]);
		if (p==NULL) continue;
		switch (*(p-1)) {
			case '[': case ' ': case ',': case '+': case '-': case '%':
				regs_target[i] = true;
				all_regs_target[i]=true;
				/// TODO: split large registers into smaller ones?
				break;
		}
	}
	smaller_to_greater_regs();	
	
}

void Finder::smaller_to_greater_regs()
{
	for(int i=16;i<20;i++)
		if (regs_target[i])
		{
			regs_target[i]=false;
			regs_target[i-8]=true;
		}
	for(int i=20;i<24;i++)
		if (regs_target[i])
		{
			regs_target[i]=false;
			regs_target[i-12]=true;
		}
	for(int i=8;i<16;i++)
		if (regs_target[i])
		{
			regs_target[i]=false;
			regs_target[i-8]=true;
		}
}

void Finder::get_operands_string(char* string1)
{
	char* p;
	if (strstr(string1,"loop")!=NULL) 
	{
		regs_target[ECX]=true;
	}
	for (int i=0; i<RegistersCount; i++)
	{
		if (regs_target[i]) continue;
		p = strstr(string1,Registers[i]);
		if (p==NULL) continue;
		switch (*(p-1)) {
			case '[': case ' ': case ',': case '+': case '-': case '%':
				regs_target[i] = true;
				/// TODO: split large registers into smaller ones?
				break;
		}
	}
	smaller_to_greater_regs();
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
					(*log) << " Indirect jump detected: " << string1 << " on position " << pos << endl;
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