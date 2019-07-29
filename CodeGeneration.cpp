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

struct genStatus
{
	stack<string> buf;
	stack<char> comBuf;
	stack<cycleInfo> cyclesInfo;
	string comReadingBuf;
	map<string, int*> numbers;
	int state;
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

void readingTextState(const char &c, genStatus &st)
{
	switch (c)
	{
	case '$':
		st.state = 1;
		break;
	default:
		st.buf.top() += c;
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

void commandExecution(genStatus &st)
{
	switch (st.comBuf.top())
	{
	case 'f':
	{
		string text = st.buf.top();
		st.buf.pop();
		for (int i = st.cyclesInfo.top().minVal; i <= st.cyclesInfo.top().maxVal; ++i)
		{
			st.buf.top().append(inputVar(text, st.cyclesInfo.top().counter, i));
		}
		st.cyclesInfo.pop();
	}
	break;
	case 'i':
	{

	}
	break;
	}
	st.comBuf.pop();
}

void readingCommandState(const char &c, genStatus &st)
{
	if (!isDelim(c) && c != '$' && c != '\n')
	{
		st.comReadingBuf += c;
	}
	else if (!st.comReadingBuf.empty())
	{
		printf("%s\n", st.comReadingBuf.c_str());
		if (st.comReadingBuf == "for")
		{
			st.comBuf.push('f');
			st.buf.push("");
			st.state = 200;
		}
		else if (st.comReadingBuf == "if")
		{
			st.comBuf.push('e');
			st.state = 300;
		}
		else if (st.comReadingBuf == "end")
		{
			commandExecution(st);
			if (c == '$' || c == '\n')
				st.state = 0;
			else
				st.state = 2;
		}
		else
		{
			//variable value search
			st.buf.top().append("$" + st.comReadingBuf + "$");
			if (c == '$' || c == '\n')
				st.state = 0;
			else
				st.state = 2;
		}
		st.comReadingBuf.clear();
	}
}

void ignoringSymbolState(const char &c, genStatus &st)
{
	if (c == '$' || c == '\n')
		st.state = 0;
}

void readingCycleCounterState(const char &c, genStatus &st)
{
	if (c == '=')
	{
		st.cyclesInfo.push(cycleInfo(st.comReadingBuf));
		st.state = 201;
		st.comReadingBuf.clear();
	}
	else if (!isDelim(c))
		st.comReadingBuf += c;
}

void readingCycleLowerBoundState(const char &c, genStatus &st)
{
	if (c == ':')
	{
		if (isDigit(st.comReadingBuf[0]) || st.comReadingBuf[0] == '-')
		{
			st.cyclesInfo.top().minVal = atoi(st.comReadingBuf.c_str());
			st.state = 202;
			st.comReadingBuf.clear();
		}
		else
		{
			//variable value search
		}
	}
	else if (!isDelim(c))
		st.comReadingBuf += c;
}

void readingCycleUpperBoundState(const char &c, genStatus &st)
{
	if (!isDelim(c) && c != '$' && c != '\n')
		st.comReadingBuf += c;
	else if (!st.comReadingBuf.empty())
	{
		if (isDigit(st.comReadingBuf[0]) || st.comReadingBuf[0] == '-')
		{
			st.cyclesInfo.top().maxVal = atoi(st.comReadingBuf.c_str());
			st.state = (c == '$') || (c == '\n') ? 0: 1;
			st.comReadingBuf.clear();
		}
		else
		{
			//variable value search
		}
	}
}

string generateCode(const string &templateFileContent)
{
	genStatus status;
	status.state = 0;
	status.buf.push("");
	for (char c : templateFileContent)
	{
		printf("state: %i\n", status.state);
		switch (status.state)
		{
		case 0: 
			readingTextState(c, status);
			break;
		case 1: 
			readingCommandState(c, status);
			break;
		case 2:
			ignoringSymbolState(c, status);
			break;
		case 200:
			readingCycleCounterState(c, status);
			break;
		case 201:
			readingCycleLowerBoundState(c, status);
			break;
		case 202:
			readingCycleUpperBoundState(c, status);
			break;
			
		}
	}

	return status.buf.top();
}

void generateCode(const char *templatePath, const char *typesSourcePath, const char *outputPath)
{
	ofstream oF = ofstream(outputPath);

	string fileContent = fileToStr(templatePath);
	oF << generateCode(fileContent).c_str();

	oF.close();
}
