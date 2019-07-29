#include "CodeGeneration.h"
#include <iostream>
#include <fstream>
#include <map>
#include <stack>
#include <string>

using namespace std;

struct cycleInfo
{
	string counter;
	int minVal;
	int maxVal;

	cycleInfo(string counterName) : minVal(0), maxVal(0)
	{
		counter = counterName;
	}
};

string fileToStr(ifstream &file)
{
	const int bufSize = 1024;
	char buf[bufSize];
	string res;

	while (file.getline(buf, bufSize - 1))
	{
		res.append(buf);
		res.append("\n");
	}

	return res;
}

string fileToStr(const char *fileName)
{
	ifstream file = ifstream(fileName);
	string res = fileToStr(file);

	file.close();
	return res;
}

bool isDelim(const char &c)
{
	return c == ' ' || c == '\n' || c == '\t';
}

bool isDigit(const char &c)
{
	return '0' <= c && c <= '9';
}

void readingTextState(const char &c, stack<string> &buf, int &state)
{
	switch (c)
	{
	case '$':
		state = 1;
		break;
	default:
		buf.top() += c;
	}
}

string inputVar(string text, string varName, int val)
{
	string sVal = to_string(val);
	bool comMode = false;
	string comBuf;
	string res;

	for (char c : text)
	{
		if (comMode)
		{
			if (c == '$' || c == '\n')
			{
				if (comBuf == varName)
					res.append(sVal);
				else
					res.append("$" + comBuf + "$");
				comBuf = "";
				comMode = false;
			}
			else if (!isDelim(c))
				comBuf += c;
		}
		else
		{
			if (c == '$')
				comMode = true;
			else
				res += c;
		}
	}

	return res;
}

void commandExecution(stack<string> &buf, stack<char> &comBuf, stack<cycleInfo> &cyclesInfo)
{
	switch (comBuf.top())
	{
	case 'f':
	{
		string text = buf.top();
		buf.pop();
		for (int i = cyclesInfo.top().minVal; i <= cyclesInfo.top().maxVal; ++i)
		{
			buf.top().append(inputVar(text, cyclesInfo.top().counter, i));
		}
		cyclesInfo.pop();
	}
	break;
	case 'i':
	{

	}
	break;
	}
	comBuf.pop();
}

void readingCommandState(const char &c, string &comReadingBuf, int &state, stack<char> &comBuf, stack<string> &buf, stack<cycleInfo> &cyclesInfo)
{
	if (!isDelim(c) && c != '$' && c != '\n')
	{
		comReadingBuf += c;
	}
	else if (!comReadingBuf.empty())
	{
		printf("%s\n", comReadingBuf.c_str());
		if (comReadingBuf == "for")
		{
			comBuf.push('f');
			buf.push("");
			state = 200;
		}
		else if (comReadingBuf == "if")
		{
			comBuf.push('e');
			state = 300;
		}
		else if (comReadingBuf == "end")
		{
			commandExecution(buf, comBuf, cyclesInfo);
			if (c == '$' || c == '\n')
				state = 0;
			else
				state = 2;
		}
		else
		{
			//variable value search
			buf.top().append("$" + comReadingBuf + "$");
			if (c == '$' || c == '\n')
				state = 0;
			else
				state = 2;
		}
		comReadingBuf.clear();
	}
}

void ignoringSymbolState(const char &c, int &state)
{
	if (c == '$' || c == '\n')
		state = 0;
}

void readingCycleCounterState(const char &c, string &comReadingBuf, int &state, map<string, int*> &counters, stack<cycleInfo> &cyclesInfo)
{
	if (c == '=')
	{
		cyclesInfo.push(cycleInfo(comReadingBuf));
		state = 201;
		comReadingBuf.clear();
	}
	else if (!isDelim(c))
		comReadingBuf += c;
}

void readingCycleLowerBoundState(const char &c, string &comReadingBuf, int &state, stack<cycleInfo> &cyclesInfo)
{
	if (c == ':')
	{
		if (isDigit(comReadingBuf[0]) || comReadingBuf[0] == '-')
		{
			cyclesInfo.top().minVal = atoi(comReadingBuf.c_str());
			state = 202;
			comReadingBuf.clear();
		}
		else
		{
			//variable value search
		}
	}
	else if (!isDelim(c))
		comReadingBuf += c;
}

void readingCycleUpperBoundState(const char &c, string &comReadingBuf, int &state, stack<cycleInfo> &cyclesInfo)
{
	if (!isDelim(c) && c != '$' && c != '\n')
		comReadingBuf += c;
	else if (!comReadingBuf.empty())
	{
		if (isDigit(comReadingBuf[0]) || comReadingBuf[0] == '-')
		{
			cyclesInfo.top().maxVal = atoi(comReadingBuf.c_str());
			state = (c == '$') || (c == '\n') ? 0: 1;
			comReadingBuf.clear();
		}
		else
		{
			//variable value search
		}
	}
}

string generateCode(const string &templateFileContent)
{
	stack<string> buf;
	stack<char> comBuf;
	stack<cycleInfo> cyclesInfo;
	string comReadingBuf;
	map<string, int*> numbers;
	int state = 0;

	buf.push("");
	for (char c : templateFileContent)
	{
		printf("state: %i\n", state, comReadingBuf.c_str());
		switch (state)
		{
		case 0: 
			readingTextState(c, buf, state);
			break;
		case 1: 
			readingCommandState(c, comReadingBuf, state, comBuf, buf, cyclesInfo);
			break;
		case 2:
			ignoringSymbolState(c, state);
			break;
		case 200:
			readingCycleCounterState(c, comReadingBuf, state, numbers, cyclesInfo);
			break;
		case 201:
			readingCycleLowerBoundState(c, comReadingBuf, state, cyclesInfo);
			break;
		case 202:
			readingCycleUpperBoundState(c, comReadingBuf, state, cyclesInfo);
			break;
			
		}
	}

	return buf.top();
}

void generateCode(const char *templatePath, const char *typesSourcePath, const char *outputPath)
{
	ofstream oF = ofstream(outputPath);

	string fileContent = fileToStr(templatePath);
	oF << generateCode(fileContent).c_str();

	oF.close();
}
