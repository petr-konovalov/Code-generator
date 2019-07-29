// Code generator.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
	const char *defTemplatePath    = "template.txt";
	const char *defTypesSourcePath = "type description.txt";
	const char *outputPath         = "generation result.txt";

	switch (argc)
	{
	case 1:
		generateCode(defTemplatePath, defTypesSourcePath, outputPath);	
		break;
	case 2:
		
		break;
	case 3:

		break;
	case 4:

		break;
	}
	getchar();
	return 0;
}
