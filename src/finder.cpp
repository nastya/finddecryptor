#include "finder.h"


using namespace std;

const int Finder::MaxCommandSize = 10;
const char* Finder::ComandsChanging [] = {"xor","add","and","or","sub","mul","imul","div","mov","pop"};
const char* Finder::Registers [] = {"eax","edx","ecx","ebx","edi","esi","ebp","esp","ax","dx","cx","bx","di","si","bp","sp",
"ah","dh","ch","bh","al","dl","cl","bl" }; 

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
	clear_data();
}

void Finder::init()
{
	RegistersCount = sizeof(Registers)/sizeof(Registers[0]);
	ComandsChangingCount = sizeof(ComandsChanging)/sizeof(ComandsChanging[0]);
	format = FORMAT_INTEL;
	data = NULL;
}
void Finder::clear_data()
{
	free(data);
	data = NULL;
}

void Finder::find() {
	INSTRUCTION inst;
	char string1[256]; 
	
	int len;
	
	for (int i = 0; i < dataSize; i++)
	{
		len = get_instruction(&inst, &data[i], MODE_32);
		if (!len || (len + i > dataSize))
		{
			continue;
		}		
		get_mnemonic_string(&inst, format,  string1, sizeof(string1));
		if (strcmp(string1,"fnstenv")==0||strcmp(string1,"call")==0||strcmp(string1,"fstenv")==0)
		{
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
	int _pos=pos; //position of seeding instruction
	vector<string> *regs_target;
	INSTRUCTION inst;
	char string1[256], string2[256]; 
	int len;
	vector <INSTRUCTION>* instructions=new vector <INSTRUCTION>;
	
	//vector <int> len_inst;
	instructions->clear();
	//len_inst.clear();
	for(; pos<dataSize; pos+=len)
	{
		len = get_instruction(&inst,&data[pos],MODE_32);
		if (!len || (len + pos > dataSize))
		{
			pos++;
			continue;
		}
		get_instruction_string(&inst, format, (DWORD)pos, string1, sizeof(string1));
		get_mnemonic_string(&inst, format,  string2, sizeof(string2));
		instructions->push_back(inst);
		//len_inst.push_back(len);
		
		for (int i=0; i<ComandsChangingCount; i++)
			if (strcmp(string2,ComandsChanging[i])==0)
			{
				for(int j=0;j<RegistersCount;j++)
				{
					if ((my_pos(string1,"[")!=-1)&&(my_pos(string1,Registers[j])>my_pos(string1,"["))
						&&(my_pos(string1,"]")>my_pos(string1,Registers[j]))&&(my_pos(string1,",")>my_pos(string1,"]")))
					{
						cout << "Write to memory detected: " << string1 << " on position " << pos << endl;
						regs_target = get_operands(pos);
						check1(instructions,regs_target);//checking whether operands of target instruction are defined
						//for(int i=0;i<regs_target->size();i++)
						//	cout<<(*regs_target)[i]<<" ";
						//cout<<endl;
						backwards_traversal(_pos,regs_target);
						for(int ii=1;ii<instructions->size();ii++)
						{
							get_instruction_string(&((*instructions)[ii]), format, 0, string1, sizeof(string1));
							cout<<string1<<endl;
						}
						delete regs_target;
						return pos;
					}
				}
			}
				
	}
	return -1;
	
}

void Finder::check1(vector <INSTRUCTION>* instructions, vector <string>* regs_target)
{
	for(int k=instructions->size()-1;k>=0;k--)
	{
		check_inst((*instructions)[k],regs_target);
	}
}

bool Finder::check2(vector <int>* num_comands, vector <int>* prev, vector <string>* regs_target)
{
	vector <INSTRUCTION>* comands=get_comands(num_comands,prev);
	check1(comands,regs_target);
	return (regs_target->size()==0);
}

vector <INSTRUCTION>* Finder::get_comands(vector <int>* num_comands,vector <int>* prev)
{
	INSTRUCTION inst;
	vector <INSTRUCTION>* comands= new vector <INSTRUCTION>;
	comands->clear();
	for(int i=num_comands->size()-1;i!=-1;i=(*prev)[i])
	{
		get_instruction(&inst,&data[(*num_comands)[i]],MODE_32);
		comands->push_back(inst);
	}
	return comands;
}

void Finder::check_inst(INSTRUCTION inst, vector <string>* regs_target)
{
	char string1[256], string2[256];
	int pos_reg, pos_comma, pos_reg2;
	get_instruction_string(&inst, format, 0, string1, sizeof(string1));
	get_mnemonic_string(&inst, format,  string2, sizeof(string2));
	for(int ii=0;ii<ComandsChangingCount;ii++)
	{
		if (strcmp(string2,ComandsChanging[ii])==0)
		{
			switch(ii)
			{
				//{"xor","add","and","or","sub","mul","imul","div","mov","pop"}
				/*xor, sub, div*/ case 0: case 4: case 7: for(int ij=0;ij<regs_target->size();ij++)
						{
							pos_reg=my_pos(string1,to_char((*regs_target)[ij]));
							pos_comma=my_pos(string1,",");
							pos_reg2=my_pos(string1,to_char((*regs_target)[ij]),pos_comma);
							if (pos_reg!=-1&&pos_comma>pos_reg&&string1[pos_reg-1]==' '&&pos_reg2!=-1)
							{
								delete_from_vector(regs_target,ij);															
							}
							for(int t=0;t<RegistersCount;t++)
							{
								pos_reg2=my_pos(string1,to_char(Registers[t]),pos_comma);
								if (pos_reg!=-1&&pos_comma>pos_reg&&string1[pos_reg-1]==' '&&pos_reg2!=-1)
								{
									regs_target->push_back(Registers[t]);
									break;
								}
							}
						}
						break;
				/*add, and, or, mul, imul*/ case 1: case 2: case 3: case 5: case 6: for(int ij=0;ij<regs_target->size();ij++)
						{
							pos_reg=my_pos(string1,to_char((*regs_target)[ij]));
							pos_comma=my_pos(string1,",");
							for(int t=0;t<RegistersCount;t++)
							{
								pos_reg2=my_pos(string1,to_char(Registers[t]),pos_comma);
								if (pos_reg!=-1&&pos_comma>pos_reg&&string1[pos_reg-1]==' '&&pos_reg2!=-1)
								{
									regs_target->push_back(Registers[t]);
									break;
								}
							}
						}
						break;
				/*mov*/ case 8: for(int ij=0;ij<regs_target->size();ij++)
						{
							pos_reg=my_pos(string1,to_char((*regs_target)[ij]));
							pos_comma=my_pos(string1,",");
							if (pos_reg!=-1&&pos_comma>pos_reg&&string1[pos_reg-1]==' ')
							{
								delete_from_vector(regs_target,ij);															
							}
						}
						break;
				/*pop*/ case 9: //cout<<"!!!Comand "<<string2<<" "<<ii<<endl;
						for(int ij=0;ij<regs_target->size();ij++)
						{
							pos_reg=my_pos(string1,to_char((*regs_target)[ij]));
							if (pos_reg!=-1&&string1[pos_reg-1]==' ')
							{
								delete_from_vector(regs_target,ij);
								regs_target->push_back("esp");
							}
						}
						break;
				
			}
			
		}
			
	}
	int j=-1;
	for(int i=0;i<regs_target->size();i++)
		if (strcmp(to_char((*regs_target)[i]),"esp")==0)
			j=i;
	if (j!=-1&&my_pos(string2,"fcmov")!=-1)
		delete_from_vector(regs_target,j);
}

void Finder::print_comands(vector <INSTRUCTION>* v)
{
	char string1[256],string2[256];
	for(int i=0;i<v->size();i++)
	{
		get_instruction_string(&((*v)[i]), format, 0, string1, sizeof(string1));
		get_mnemonic_string(&((*v)[i]), format,  string2, sizeof(string2));
		cout<<string1<<endl;
	}
}

void Finder::backwards_traversal(int pos, vector <string>* regs_target)
{
	INSTRUCTION inst;
	int len;
	char string1[256];
	int cur=0;
	int length=1;
	vector <int>* queue=new vector <int>;
	vector <int>* prev=new vector <int>;
	vector <string> copy_of_regs;
	for(int i=0;i<regs_target->size();i++)
		copy_of_regs.push_back((*regs_target)[i]);
	queue->clear();
	prev->clear();
	queue->push_back(pos);
	prev->push_back(-1);
	while(cur<length)
	{
		for(int i=1;i<=MaxCommandSize;i++)
		{
			len = get_instruction(&inst,&data[(*queue)[cur]-i],MODE_32);
			if (len==i)
			{
				queue->push_back((*queue)[cur]-i);
				prev->push_back(cur);
				length++;
				if (check2(queue,prev,regs_target))
				{
					length=0;
					break;
				}
				else
				{
					regs_target->clear();
					for(int j=0;j<copy_of_regs.size();j++)
						regs_target->push_back(copy_of_regs[j]);
				}
			}
		}
		cur++;
	}
	vector <INSTRUCTION>* instructions=get_comands(queue,prev); 
	print_comands(instructions);
}

void Finder::delete_from_vector(vector <string>* regs_target, int ij)
{
	for(int ik=ij;ik<regs_target->size()-1;ik++)
		(*regs_target)[ik]=(*regs_target)[ik+1];
	regs_target->pop_back();
}

char* Finder::to_char(string s)
{
	int i;
	char *str=new char [s.length()+1];
	for(i=0;i<s.length();i++)
		str[i]=s[i];
	str[i]='\0';
	return str;
}

vector<string> * Finder::get_operands(int pos)
{
	vector<string> *regs_target = new vector<string>;
	regs_target->clear();
	INSTRUCTION inst;
	get_instruction(&inst,&data[pos],MODE_32);
	char string1[256];
	int _pos;
	get_instruction_string(&inst,format,(DWORD)pos,string1,sizeof(string1));
	for(int i=0; i<RegistersCount; i++)
	{
		_pos=my_pos(string1,Registers[i]);
		if (_pos!=-1&&(string1[_pos-1]=='['||string1[_pos-1]==' '||string1[_pos-1]==','||string1[_pos-1]=='+'||string1[_pos-1]=='-'))
		{
			regs_target->push_back(Registers[i]);
			//switch(i)
			//{
			//	/*ax*/case 8: regs_target->push_back("ah");
			//		      regs_target->push_back("al"); break;
			//	/*dx*/case 9: regs_target->push_back("dh");
			//		      regs_target->push_back("dl"); break;
			//	/*cx*/case 10: regs_target->push_back("ch");
			//		       regs_target->push_back("cl"); break;
			//	/*bx*/case 11: regs_target->push_back("bh");
			//		       regs_target->push_back("bl"); break;
			//	      default: regs_target->push_back(Registers[i]);
				
			//}			
			
			//cout<<Registers[i]<<" ";
		}
		
			
	}
	//clear same registers
	//for(int i=0;i<regs_target->size();i++)
	//	cout<<(*regs_target)[i]<<" ";
	//cout<<endl;
	return regs_target;
}

int Finder::find_jump(int pos)
{
	INSTRUCTION inst;
	char string1[256], string2[256]; 
	int len;
	
	for(; pos<dataSize; pos+=len)
	{
		len = get_instruction(&inst,&data[pos],MODE_32);
		if (!len || (len + pos > dataSize))
		{
			pos++;
			continue;
		}
		get_instruction_string(&inst, format, (DWORD)pos, string1, sizeof(string1));
		get_mnemonic_string(&inst, format,  string2, sizeof(string2));
		
		if (string2[0]=='j')
		{
			for (int i=0; i<RegistersCount; i++)
				if ((my_pos(string1," ")!=-1)&&(my_pos(string1,Registers[i])>my_pos(string1," ")))
				{
					cout << "Indirect jump detected: " << string1 << " on position " << pos << endl;
					get_operands(pos);
					return pos;
				}
		}
	}
	return -1;
	
}

int Finder::my_pos(char* s, const char *p, int start) //naive
{
	bool ok;
	int j,i1;
	for(int i=start; s[i] != '\0'; i++)
	{
		ok=true;
		for(j=0,i1=i; p[j]!='\0';j++,i1++)
		{
			if (s[i1]!=p[j])
			{
				ok=false;
				break;
			}
		}
		if (ok)
			return i;
	}
	return -1;
}
/*
int Finder::my_pos(char* s, const char *p, int start) {
	char *pos = strstr(s+start, p);
	return (pos==NULL)?-1:((int) (pos-s));
}
*/

void Finder::read_file(char *name)
{
	clear_data();
	FILE *fp;
	int c;
	struct stat sstat;

	if ((fp = fopen(name, "r+b")) == NULL)
	{
		cerr << "Error: unable to open file \"" << name << "\"" << endl;
		exit(0);
	}

	/* Get file len */
	if ((c = stat(name, &sstat)) == -1)
	{
		cerr << "Error: stat" << endl;
		exit (1);
	}

	dataSize = sstat.st_size;
	data = new unsigned char[dataSize];

	/* Read file in allocated space */
	if ((c = fread(data, 1, dataSize, fp)) != dataSize)
	{
		cerr << "Error: fread" << endl;
		exit (1);
	}

	fclose(fp);
} 
