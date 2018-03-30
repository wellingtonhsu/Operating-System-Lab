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
	int offset;
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


int main()
{
	ifstream myReadFile;
 	myReadFile.open("input-10");
	vector<Symbol> defList;
	vector<string> useList;
	vector<Text> programText;
	set<string> multiDef;

	int module = 1;
	int moduleCount = 1;
	int address = 0;
	int noDef=0;
	int noUse=0;
	int noText=0;
	int value;
	string token;


	// Pass One
	if (myReadFile.is_open())
	{
		while (!myReadFile.eof())
		{

			//Read Definition List
			if (module == 1)
			{
				myReadFile >> noDef;
				if (!myReadFile.eof()){
					for (int i = 1; i <= noDef; i++)
					{
						myReadFile >> token;
						myReadFile >> value;
						//Check if defined multiple times
						if (symbolTable.find(token) != symbolTable.end())
						{
							multiDef.insert(token);
						}
						symbolTable.insert(pair<string, int>(token, address+value));
						symbolModule.insert(pair<string, int>(token, moduleCount));
					}
				}
				module = 2;
			}

			//Read User List
			else if (module == 2)
			{
				myReadFile >> noUse;
				for (int i = 1; i <= noUse; i++)
				{
					myReadFile >> token;
				}
				module = 3;
			}

			//Read Program Text
			else
			{
				myReadFile >> noText;
				for (int i = 1; i <= noText; i++)
				{
					myReadFile >> token >> token;
				}

				for(map<string, int>::iterator iter = symbolTable.begin(); iter != symbolTable.end(); iter++)
				{
					if(iter->second > noText)
					{
						iter->second = address;
						cout << "Warning: Module "<< moduleCount<<": "<<iter->first<<" too big "<<value<<" (max="<<noText-1<<") assume zero relative"<< endl;
					}
				}
				module = 1;
				moduleCount += 1;
				address = noText + address;
			}
		}
	}

	myReadFile.close();

	//Print Symbol Table
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


	//////////////////////////////////////////////////////////////////
	//Pass Two

	myReadFile.open("input-10");
	map<int, string> error;
	map<int, string> warning;
	vector<int> MemoryMap;
	vector<string> usedSymbol;
	set<string> useListFlag;
	int relativeAddr = 0;
	int noUseList;
	module = 1;
	moduleCount = 1;

	if (myReadFile.is_open())
	{
		while (!myReadFile.eof())
		//Read Definition List
		if (module == 1)
		{
			myReadFile >> noDef;
			if (!myReadFile.eof()){
				for (int i = 1; i <= noDef; i++)
				{
					myReadFile >> token >> token;
				}
			}
			module = 2;
		}

		//Read User List
		else if (module == 2)
		{
			myReadFile >> noUse;
			for (int i = 1; i <= noUse; i++)
			{
				myReadFile >> token;
				useList.push_back(token);
				usedSymbol.push_back(token);
			}
			module = 3;
		}

		//Read Program Text
		else
		{
			myReadFile >> noDef;
			for (int i = 1; i <= noDef; i++)
			{
				Text memory;
				myReadFile >> token;
				memory.name = token;
				myReadFile >> token;
				memory.Addr = stoi(token);
				memory.offset = relativeAddr;
				programText.push_back(memory);
			}
			module = 1;
			relativeAddr = relativeAddr + noDef;

			//Build Memory Table (make it into a function!)
			for (int i=0; i < programText.size(); i++)
			{
				if (programText[i].name == "I")
				{
					if (programText[i].Addr > 9999)
					{
						error.insert(pair<int, string>(i+programText[i].offset, " Error: Illegal immediate value; treated as 9999"));
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
						error.insert(pair<int, string>(i+programText[i].offset, " Error: Illegal opcode; treated as 9999"));
						MemoryMap.push_back(9999);
					}
					else if (programText[i].Addr%1000 > 512)
					{
						error.insert(pair<int, string>(i+programText[i].offset, " Error: Absolute address exceeds machine size; zero used"));
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
						error.insert(pair<int, string>(i+programText[i].offset, " Error: Illegal opcode; treated as 9999"));
						MemoryMap.push_back(9999);
					}
					else
					{
						programText[i].Addr = programText[i].Addr + programText[i].offset;
						if (programText[i].Addr%1000 > noDef)
						{
							error.insert(pair<int, string>(i+programText[i].offset, " Error: Relative address exceeds module size; zero used"));
							MemoryMap.push_back(programText[i].Addr/1000*1000 + programText[i].offset);
						}
						else
						{
							MemoryMap.push_back(programText[i].Addr);
						}
					}
				}
				else
				{
					if (programText[i].Addr > 9999)
					{
						error.insert(pair<int, string>(i+programText[i].offset, " Error: Illegal opcode; treated as 9999"));
						MemoryMap.push_back(9999);
					}
					else if(programText[i].Addr%1000 > noUse-1 )
					{
						error.insert(pair<int, string>(i+programText[i].offset, " Error: External address exceeds length of uselist; treated as immediate"));
						MemoryMap.push_back(programText[i].Addr);
					}
					else if(symbolTable.find(useList[programText[i].Addr%1000]) == symbolTable.end())
					{
						error.insert(pair<int, string>(i+programText[i].offset, " Error: "+useList[programText[i].Addr%1000]+" is not defined; zero used"));
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
			//Find the symbols not used from used list
			for (int k=0; k<useList.size(); k++)
			{
				if(useListFlag.find(useList[k]) == useListFlag.end())
				{
					warning.insert(pair<int, string>(programText.size()-1, "Warning: Module "+to_string(moduleCount)+": "+useList[k] + " appeared in the uselist but was not actually used"));
				}
			}
			//Clear useList and programText for
			useList.clear();
			programText.clear();
			moduleCount += 1;
		}
	}

	//Print Memory Table
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
	cout << endl;


	//Rule 4 Check
	//Should converst used symbol from vector to a set?
	//Throw whole error message into the set?
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
			cout << "Warning: Module "<< iter->second <<": "<< iter->first << " was defined but never used" << endl;
		}
	}
}
