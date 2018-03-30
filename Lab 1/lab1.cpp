#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>
#include <map>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <iomanip>
#include <iterator>
#include <string>
#include <set>
#include <ctype.h>

using namespace std;

struct Symbol
{
	string name;
	int Addr;
};

struct Text
{
	string name;
	int Addr;
	int AddrOffset;
};

struct catchToken
{
	string name;
	int offset;
	int lineNo;
};

map<string, int> symbolTable;
map<string, int> symbolModule;
void parseerror(int linenum, int lineoffset, int errcode) {
	static string errstr[] = {
			"NUM_EXPECTED",
			"SYM_EXPECTED",
			"ADDR_EXPECTED",
			"SYM_TOLONG",
			"TO_MANY_DEF_IN_MODULE",
			"TO_MANY_USE_IN_MODULE",
			"TO_MANY_INSTR"
	};
	cout<<"Parse Error line " << linenum << " offset " << lineoffset <<": "<<  errstr[errcode] << endl;
}

void printMemTable();
void printWarning();
void printSymTable();
void buildMemTable();
string toString (int Number);

vector<string> useList;
vector<catchToken> tokens;
vector<Text> programText;
set<string> multiDef;
vector<int> MemoryMap;
vector<int> charPosition;
map<int, string> error;
map<int, string> warning;
vector<string> usedSymbol;
set<string> useListFlag;

int moduleCount = 1;
int address = 0;
int noDef=0;
int noUse=0;
int noText=0;
int useNo=0;
int lineNum=1;
int state=0;
int readIn=0;
int TextNo=0;
int endOf=0;
int totalprogramText=0;
int totaluseList=0;
int totaldefList=0;
int value;
int flag;

string temp;
string line;


int main(int arg, char* argv[])
{

	ifstream myReadFile;
	if (argv[1] == NULL)
	{
		cout<<"Error, no input";
		exit (EXIT_FAILURE);
	}
 	myReadFile.open(argv[1]);


	// Read File
	if (myReadFile.is_open())
	{
		while (getline(myReadFile, line))
		{
			std::size_t found = line.find_first_of(" \t");
			int position = 0;
			while (found != std::string::npos)
			{
				if (line.substr(position, found-position) != "")
				{
					catchToken tmp;
					tmp.name = line.substr(position, found-position);
					tmp.offset = position;
					tmp.lineNo = lineNum;
					tokens.push_back(tmp);
					endOf = position+ tmp.name.size();
				}
				position = found+1;
				found = line.find_first_of(" \t",found+1);
			}
			if (line.substr(position) != "")
			{
				catchToken tmp;
				tmp.name = line.substr(position);
				tmp.offset = position;
				tmp.lineNo = lineNum;
				tokens.push_back(tmp);
				endOf = position+ tmp.name.size();
			}
			lineNum += 1;
			if (position == 0 && tokens[tokens.size()-1].lineNo != lineNum-1)
			{
				endOf = 0;
				tokens[tokens.size()-1].lineNo = lineNum - 1;
			}

		}
		myReadFile.close();
	}

	//Pass One
	for(int i=0; i<tokens.size();i++)
	{
		//Read defList
		if(state == 0)
		{
			if(readIn == 0)
			{
				if (isdigit(tokens[i].name.at(0)) == 0)
				{
					parseerror(tokens[i].lineNo, endOf+1, 0);
					exit (EXIT_FAILURE);
				}

				noDef = (stoi(tokens[i].name))*2;
				totaldefList += noDef;
				if (i == tokens.size()-1 && i <= totaldefList+totaluseList+totalprogramText)
				{
					parseerror(tokens[i].lineNo, endOf+1, 1);
					exit (EXIT_FAILURE);
				}
				if (stoi(tokens[i].name) > 16)
				{
					parseerror(tokens[i].lineNo, tokens[i].offset+1, 4);
					exit (EXIT_FAILURE);
				}
				if(noDef == 0)
				{
					state = 1 ;
				}
				else
				{
					readIn = 1;
				}
			}
			else
			{
				if (noDef%2 == 0)
				{
					if (isdigit(tokens[i].name.at(0)) == 1)
					{
						parseerror(tokens[i].lineNo, tokens[i].offset+1, 1);
						exit (EXIT_FAILURE);
					}
					temp = tokens[i].name;

					if (i == tokens.size()-1 && i <= totaldefList+totaluseList+totalprogramText)
					{
						//cout<<i<<endl;
						//cout<<tokens.size()<<endl;
						//cout<<totaldefList+totaluseList+totalprogramText<<endl;
						parseerror(tokens[i].lineNo, endOf+1, 0);
						exit (EXIT_FAILURE);
					}
				}
				else
				{
					if (isdigit(tokens[i].name.at(0)) == 0)
					{
						parseerror(tokens[i].lineNo, tokens[i].offset+1, 0);
						exit (EXIT_FAILURE);
					}

					if (i == tokens.size()-1 && i < totaldefList+totaluseList+totalprogramText)
					{
						parseerror(tokens[i].lineNo, endOf+1, 1);
						exit (EXIT_FAILURE);
					}

					if (symbolTable.find(temp) != symbolTable.end())
					{
						multiDef.insert(temp);
					}
					value = stoi(tokens[i].name);
					symbolTable.insert(pair<string, int>(temp, address+value));
					symbolModule.insert(pair<string, int>(temp, moduleCount));
				}
				noDef -= 1;
				if (noDef <= 0)
				{
					state = 1;
					readIn = 0;
				}
			}
		}

		//UseList
		else if (state == 1)
		{
			if(readIn == 0)
			{
				if (isdigit(tokens[i].name.at(0)) == 0)
				{
					parseerror(tokens[i].lineNo, endOf+1, 0);
					exit (EXIT_FAILURE);
				}
				noUse = stoi(tokens[i].name);
				totaluseList += noUse;

				if (i == tokens.size()-1 && i <= totaldefList+totaluseList+totalprogramText)
				{
					parseerror(tokens[i].lineNo, endOf+1, 1);
					exit (EXIT_FAILURE);
				}

				if (noUse > 16)
				{
					parseerror(tokens[i].lineNo, tokens[i].offset+1, 5);
					exit (EXIT_FAILURE);
				}
				if(noUse == 0)
				{
					state = 2;
				}
				else
				{
					readIn = 1;
				}
			}
			else
			{
				if (isdigit(tokens[i].name.at(0)) == 1)
				{
					parseerror(tokens[i].lineNo, tokens[i].offset+1, 1);
					exit (EXIT_FAILURE);
				}
				if (i == tokens.size()-1 && i <= totaldefList+totaluseList+totalprogramText)
				{
					parseerror(tokens[i].lineNo, endOf+1, 1);
					exit (EXIT_FAILURE);
				}
				noUse -= 1;
				if (noUse <= 0)
				{
					state = 4;
					readIn = 0;
				}
			}
		}

		//ProgramText
		else
		{
			if (readIn == 0)
			{
				if (isdigit(tokens[i].name.at(0)) == 0)
				{
					parseerror(tokens[i].lineNo, endOf+1, 0);
					exit (EXIT_FAILURE);
				}
				noText = (stoi(tokens[i].name))*2;
				TextNo = stoi(tokens[i].name);
				totalprogramText += noText;
				if (i == tokens.size()-1 && i <= totaldefList+totaluseList+totalprogramText)
				{
					parseerror(tokens[i].lineNo, endOf+1, 2);
					exit (EXIT_FAILURE);
				}

				if (totalprogramText > 512)
				{
					parseerror(tokens[i].lineNo, tokens[i].offset+1, 6);
					exit (EXIT_FAILURE);
				}
				if(noText == 0)
				{
					state = 0;
				}
				else
				{
					readIn = 1;
				}
			}
			else
			{
				if (noText%2 == 0)
				{
					if(tokens[i].name != "A" && tokens[i].name != "E" && tokens[i].name != "I" && tokens[i].name != "R")
					{
						parseerror(tokens[i].lineNo, tokens[i].offset+1, 2);
						exit (EXIT_FAILURE);
					}
					if (i == tokens.size()-1 && i <= totaldefList+totaluseList+totalprogramText)
					{
						parseerror(tokens[i].lineNo, endOf+1, 0);
						exit (EXIT_FAILURE);
					}
				}
				else
				{
					if (isdigit(tokens[i].name.at(0)) == 0)
					{
						parseerror(tokens[i].lineNo, tokens[i].offset+1, 0);
						exit (EXIT_FAILURE);
					}
					if (i == tokens.size()-1 && i <= totaldefList+totaluseList+totalprogramText)
					{
						parseerror(tokens[i].lineNo, endOf+1, 2);
						exit (EXIT_FAILURE);
					}
				}

				noText -= 1;
				if (noText <= 0)
				{
					for(map<string, int>::iterator iter = symbolTable.begin(); iter != symbolTable.end(); iter++)
					{
						if(iter->second - address > TextNo)
						{
							iter->second = address;
							cout << "Warning: Module "<< moduleCount<<": "<<iter->first<<" too big "<<value<<" (max="<<TextNo-1<<") assume zero relative"<< endl;
						}
					}
					state = 0;
					readIn = 0;
					moduleCount += 1;
					address += TextNo;
				}
			}
		}
	}

	//Print Symbol Table
	printSymTable();

	//Pass Two

	int relativeAddr = 0;
	int noUseList;
	state = 0;
	moduleCount = 1;
	Text memory;

	for(int i=0; i<tokens.size();i++)
	{
		if(state == 0)
		{
			if(readIn == 0)
			{
				noDef = (stoi(tokens[i].name))*2;
				if(noDef == 0)
				{
					state = 1 ;
				}
				else
				{
					readIn = 1;
				}
			}
			else
			{
				noDef -= 1;
				if (noDef <= 0)
				{
					state = 1;
					readIn = 0;
				}
			}
		}

		//Read UseList
		else if (state == 1)
		{
			if(readIn == 0)
			{
				noUse = stoi(tokens[i].name);
				useNo = stoi(tokens[i].name);
				if(noUse == 0)
				{
					state = 2;
				}
				else
				{
					readIn = 1;
				}
			}
			else
			{
				useList.push_back(tokens[i].name);
				usedSymbol.push_back(tokens[i].name);
				noUse -= 1;
				if (noUse <= 0)
				{
					state = 4;
					readIn = 0;
				}
			}
		}

		//Read ProgramText
		else
		{
			if (readIn == 0)
			{
				noText = (stoi(tokens[i].name))*2;
				TextNo = stoi(tokens[i].name);
				if(noText == 0)
				{
					state = 0;
				}
				else
				{
					readIn = 1;
				}
			}
			else
			{
				if(noText % 2 == 0)
				{
					memory.name = tokens[i].name;
				}
				else
				{
					memory.Addr = stoi(tokens[i].name);
					memory.AddrOffset = relativeAddr;
					programText.push_back(memory);
				}
				noText -= 1;
				if (noText <= 0)
				{
					state = 0;
					readIn = 0;
					buildMemTable();

					//Find the symbols not used from used list
					for (int k=0; k<useList.size(); k++)
					{
						if(useListFlag.find(useList[k]) == useListFlag.end())
						{
							warning.insert(pair<int, string>(programText.size()-1, "Warning: Module "+ toString(moduleCount)+": "+ useList[k] + " appeared in the uselist but was not actually used"));
						}
					}
					//Clear useList and programText for
					relativeAddr = relativeAddr + TextNo;
					useList.clear();
					programText.clear();
					moduleCount += 1;
				}
			}
		}
	}
	printMemTable();
	printWarning();
}




string toString (int Number )
{
 ostringstream ss;
 ss << Number;
 return ss.str();
}

void printSymTable()
{
	cout << "Symbol Table" << endl;
	for(map<string, int>::iterator iter = symbolTable.begin(); iter != symbolTable.end(); iter++)
	{
		cout << iter->first << "=" << iter->second;
		//Check for Rule 2
		if (multiDef.find(iter->first) != multiDef.end())
		{
			cout << " Error: This variable is multiple times defined; first value used";
		}
		cout <<endl;
	}
	cout << endl;
}

void buildMemTable()
{
	for (int i=0; i < programText.size(); i++)
	{
		if (programText[i].name == "I")
		{
			if (programText[i].Addr > 9999)
			{
				error.insert(pair<int, string>(i+programText[i].AddrOffset, " Error: Illegal immediate value; treated as 9999"));
				MemoryMap.push_back(9999);
			}
			else
			{
				MemoryMap.push_back(programText[i].Addr);
			}
		}
		else if (programText[i].name == "A")
		{
			if (programText[i].Addr > 9999)
			{
				error.insert(pair<int, string>(i+programText[i].AddrOffset, " Error: Illegal opcode; treated as 9999"));
				MemoryMap.push_back(9999);
			}
			else if (programText[i].Addr%1000 > 512)
			{
				error.insert(pair<int, string>(i+programText[i].AddrOffset, " Error: Absolute address exceeds machine size; zero used"));
				MemoryMap.push_back(programText[i].Addr/1000*1000);
			}
			else
			{
				MemoryMap.push_back(programText[i].Addr);
			}
		}
		else if (programText[i].name == "R")
		{
			if (programText[i].Addr > 9999)
			{
				error.insert(pair<int, string>(i+programText[i].AddrOffset, " Error: Illegal opcode; treated as 9999"));
				MemoryMap.push_back(9999);
			}
			else
			{
				if (programText[i].Addr%1000 > TextNo)
				{
					error.insert(pair<int, string>(i+programText[i].AddrOffset, " Error: Relative address exceeds module size; zero used"));
					MemoryMap.push_back(programText[i].Addr/1000*1000 + programText[i].AddrOffset);
				}
				else
				{
					programText[i].Addr = programText[i].Addr + programText[i].AddrOffset;
					MemoryMap.push_back(programText[i].Addr);
				}
			}
		}
		else
		{
			if (programText[i].Addr > 9999)
			{
				error.insert(pair<int, string>(i+programText[i].AddrOffset, " Error: Illegal opcode; treated as 9999"));
				MemoryMap.push_back(9999);
			}
			else if(programText[i].Addr%1000 > useNo-1 )
			{
				error.insert(pair<int, string>(i+programText[i].AddrOffset, " Error: External address exceeds length of uselist; treated as immediate"));
				MemoryMap.push_back(programText[i].Addr);
			}
			else if(symbolTable.find(useList[programText[i].Addr%1000]) == symbolTable.end())
			{
				error.insert(pair<int, string>(i+programText[i].AddrOffset, " Error: "+useList[programText[i].Addr%1000]+" is not defined; zero used"));
				MemoryMap.push_back(programText[i].Addr/1000*1000);
			}
			else
			{
				MemoryMap.push_back(programText[i].Addr/1000*1000 + symbolTable.find(useList[programText[i].Addr%1000])->second);
			}
			//Flag the symbol used in used list
			for (int k=0; k<useList.size(); k++)
			{
				if(programText[i].Addr%1000 == k)
				{
					useListFlag.insert(useList[k]);
				}
			}
		}
	}
}

void printMemTable()
{
	cout << "Memory Map" << endl;
	for (int i=0; i < MemoryMap.size(); i++)
	{
		cout << setfill('0') << setw(3) << i << ": " << setfill('0') << setw(4) << MemoryMap[i];
		if (error.find(i) != error.end())
		{
			cout << error.find(i)->second;
		}
		if (warning.find(i) != warning.end())
		{
			cout << endl << warning.find(i)->second;
		}
		cout << endl;
	}
}

void printWarning()
{
	//Print Warning Messages
	for(map<string, int>::iterator iter = symbolModule.begin(); iter != symbolModule.end(); iter++)
	{
		int flag = 0;
		for (int j=0; j<usedSymbol.size(); j++)
		{
			if (iter->first == usedSymbol[j])
			{
				flag = 1;
			}
		}
		if (flag == 0)
		{
			cout << endl;
			cout << "Warning: Module "<< iter->second <<": "<< iter->first << " was defined but never used" << endl;
		}
	}
}
