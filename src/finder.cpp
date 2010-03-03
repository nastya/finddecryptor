#include "finder.h"


using namespace std;

const char* Finder::ComandsChanging [] = {"xor","add","and","or","sub","mul","imul","div","mov"};
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
	vector<string> *regs_target;
	INSTRUCTION inst;
	char string1[256], string2[256]; 
	int len;
	vector <INSTRUCTION> instructions;
	int pos_reg, pos_comma;
	//vector <int> len_inst;
	instructions.clear();
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
		instructions.push_back(inst);
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
						/*for(int ii=0;ii<instructions.size();ii++)
						{
							get_instruction_string(&instructions[ii], format, 0, string1, sizeof(string1));
							cout<<string1<<endl;
						}*/
						//checking whether operands of target instruction are defined
						for(int k=instructions.size();k>=0;k--)
						{
							get_instruction_string(&inst, format, (DWORD)pos, string1, sizeof(string1));
							get_mnemonic_string(&inst, format,  string2, sizeof(string2));
							for(int ii=0;ii<ComandsChangingCount;ii++)
							{
								if (strcmp(string2,ComandsChanging[ii])!=-1)
								{
									switch(ii)
									{
										/*mov*/ case 8: for(int ij=0;ij<regs_target->size();ij++)
												{
													pos_reg=my_pos(string1,to_char((*regs_target)[ij]));
													pos_comma=my_pos(string1,",");
													if (pos_reg!=-1&&pos_comma>pos_reg&&string1[pos_reg-1]==' ')
													{
														for(int ik=ij;ik<regs_target->size()-1;ik++)
															(*regs_target)[ik]=(*regs_target)[ik+1];
														regs_target->pop_back();
															
													}
												}
												break;
									}
									
								}
									
							}
						}
						for(int i=0;i<regs_target->size();i++)
							cout<<(*regs_target)[i]<<" ";
						cout<<endl;
						delete regs_target;
						return pos;
					}
				}
			}
				
	}
	return -1;
	
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
			switch(i)
			{
				/*ax*/case 8: regs_target->push_back("ah");
					      regs_target->push_back("al"); break;
				/*dx*/case 9: regs_target->push_back("dh");
					      regs_target->push_back("dl"); break;
				/*cx*/case 10: regs_target->push_back("ch");
					       regs_target->push_back("cl"); break;
				/*bx*/case 11: regs_target->push_back("bh");
					       regs_target->push_back("bl"); break;
				      default: regs_target->push_back(Registers[i]);
				
			}			
			
			//cout<<Registers[i]<<" ";
		}
		
			
	}
	//clear same registers
	for(int i=0;i<regs_target->size();i++)
		cout<<(*regs_target)[i]<<" ";
	cout<<endl;
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

int Finder::my_pos(char* s, const char *p) //naive
{
	bool ok;
	int j,i1;
	for(int i=0; s[i] != '\0'; i++)
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
