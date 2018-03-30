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


int main()
{
	ifstream myReadFile;
 	myReadFile.open("input-1");
	vector<Symbol> defList;
	vector<string> useList;
	vector<Text> programText;
	set<string> multiDef;
	int moduleCount = 1;
	int address = 0;
	int noDef=0;
	int noUse=0;
	int noText=0;
	int lineNum=1;
	int state=0;
	int read=0;
	int checker;
	int value;

	string token;
	string temp;
	string line;

	//State 0 = In defList
	//State 1 = In readList
	//State 2 = In programText
	//Read 0 = reading number of input
	//Read 1 = reading in tokens

	// Pass One
	if (myReadFile.is_open())
	{
		while (getline(myReadFile, line))
		{
			istringstream substring(line);
      while (substring >> token)
			{
				//Read defList
				if(state == 0)
				{
					if(read == 0)
					{
						noDef = (stoi(token))*2;
						if(noDef == 0)
						{
							state = 1 ;
						}
						else
						{
							read = 1;
						}
					}
					else
					{
						if (noDef%2 == 0)
						{
							temp = token;
						}
						else
						{
							if (symbolTable.find(temp) != symbolTable.end())
							{
								multiDef.insert(temp);
							}
							value = stoi(token);
							symbolTable.insert(pair<string, int>(temp, address+value));
							symbolModule.insert(pair<string, int>(temp, moduleCount));
						}
						noDef -= 1;
						if (noDef <= 0)
						{
							state = 1;
							read = 0;
						}
					}
				}

				//UseList
				else if (state == 1)
				{
					if(read == 0)
					{
						noUse = stoi(token);
						if(noUse == 0)
						{
							state = 2;
						}
						else
						{
							read = 1;
						}
					}
					else
					{
						noUse -= 1;
						if (noUse <= 0)
						{
							state = 4;
							read = 0;
						}
					}
				}

				//ProgramText
				else
				{
					if (read == 0)
					{
						noText = (stoi(token))*2;
						checker = stoi(token);
						address += stoi(token);
						if(noText == 0)
						{
							state = 0;
						}
						else
						{
							read = 1;
						}
					}
					else
					{
						noText -= 1;
						if (noText <= 0)
						{

							state = 0;
							read = 0;
							moduleCount += 1;
						}
					}
				}
			}
			lineNum += 1;
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
	}

	//////////////////////////////////////////////////////////////////////////////
	//Pass Two
	myReadFile.open("input-1");
	map<int, string> error;
	map<int, string> warning;
	vector<int> MemoryMap;
	vector<string> usedSymbol;
	set<string> useListFlag;
	int relativeAddr = 0;
	int noUseList;
	state = 0;
	moduleCount = 1;
	Text memory;

	if (myReadFile.is_open())
	{
		while (getline(myReadFile, line))
		{
			istringstream substring(line);
      while (substring >> token)
			{
				//Read defList
				if(state == 0)
				{
					if(read == 0)
					{
						noDef = (stoi(token))*2;
						if(noDef == 0)
						{
							state = 1 ;
						}
						else
						{
							read = 1;
						}
					}
					else
					{
						noDef -= 1;
						if (noDef <= 0)
						{
							state = 1;
							read = 0;
						}
					}
				}

				//UseList
				else if (state == 1)
				{
					if(read == 0)
					{
						noUse = stoi(token);
						if(noUse == 0)
						{
							state = 2;
						}
						else
						{
							read = 1;
						}
					}
					else
					{
						useList.push_back(token);
						usedSymbol.push_back(token);
						noUse -= 1;
						if (noUse <= 0)
						{
							state = 4;
							read = 0;
						}
					}
				}

				//ProgramText
				else
				{
					if (read == 0)
					{
						noText = (stoi(token))*2;
						address += stoi(token);
						if(noText == 0)
						{
							state = 0;
						}
						else
						{
							read = 1;
						}
					}
					else
					{
						if(noText % 2 == 0)
						{
							memory.name = token;
						}
						else
						{
							memory.Addr = stoi(token);
							memory.offset = relativeAddr;
							programText.push_back(memory);
						}
						noText -= 1;
						if (noText <= 0)
						{
							state = 0;
							read = 0;
							moduleCount += 1;
							relativeAddr = relativeAddr + (noText/2);
						}

					}
				}
			}
			lineNum += 1;
		}
		myReadFile.close();




	}
}
