#include "CodeGeneration.h"

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
	return c != ' ' && c != '\n' && c != '\t';
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

void readingCommandState(const char &c, string &comReadingBuf, int &state)
{
	if (!isDelim(c))
		comReadingBuf += c;
	else if (c == '$')
		state = 0;
	else if (!comReadingBuf.empty())
	{
		if (comReadingBuf == "for")
			state = 200;
		else if (comReadingBuf == "if")
			state = 300;
		else if (comReadingBuf == "end")
			state = 400;
		else
			state = 500;
		comReadingBuf.clear();
	}
}

void readingCommandForCounterState(const char &c, string &comReadingBuf, int &state, map<string, int*> &counters, stack<pair<int*, int> > cyclesBuf)
{
	if (!isDelim(c))
		comReadingBuf += c;
	else if (c == '=')
	{
		cyclesBuf.push(make_pair(new int, 0));
		counters[comReadingBuf] = cyclesBuf.top().first;
		state = 201;
		comReadingBuf.clear();
	}
}

void readingCommandForLowerBoundState(const char &c, string &comReadingBuf, int &state, stack<pair<int*, int> > cyclesBuf)
{
	if (!isDelim(c))
	{
		comReadingBuf += c;
	}
	else if (c == ':')
	{
		int value = atoi(comReadingBuf.c_str());
		*cyclesBuf.top().first = value;
		state = 202;
		comReadingBuf.clear();
	}
}

void readingCommandForUpperBoundState(const char &c, string &comReadingBuf, int &state, stack<pair<int*, int> > cyclesBuf)
{
	if (!isDelim(c))
	{
		comReadingBuf += c;
	}
	else if (!comReadingBuf.empty())
	{
		int value = atoi(comReadingBuf.c_str());
		cyclesBuf.top().second = value;
		state = 1;
		comReadingBuf.clear();
	}
}

string generateCode(const string &templateFileContent)
{
	stack<string> buf;
	stack<string> comBuf;
	stack<pair<int*, int> > cyclesBuf;
	string comReadingBuf;
	map<string, int*> numbers;
	int state = 0;

	buf.push("");
	for (char c : templateFileContent)
	{
		switch (state)
		{
		case 0: 
			readingTextState(c, buf, state);
			break;
		case 1: 
			readingCommandState(c, comReadingBuf, state);
			break;
		case 200:
			readingCommandForCounterState(c, comReadingBuf, state, numbers, cyclesBuf);
			break;
		case 201:
			readingCommandForLowerBoundState(c, comReadingBuf, state, cyclesBuf);
			break;
		case 202:
			readingCommandForUpperBoundState(c, comReadingBuf, state, cyclesBuf);
			break;
		}
	}

	return buf.top();
}

void generateCode(const char *templateFileName, const char *typesDescriptionFileName, const char *outputFileName)
{
	ofstream oF = ofstream(outputFileName);

	string fileContent = fileToStr(templateFileName);
	oF << generateCode(fileContent).c_str();

	oF.close();
}
