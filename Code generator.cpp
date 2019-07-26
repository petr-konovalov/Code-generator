// Code generator.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
	const char *defTemplateFileName         = "template.txt";
	const char *defTypesDescriptionFileName = "type description.txt";
	const char *outputFileName              = "generation result.txt";

	switch (argc)
	{
	case 1:
		generateCode(defTemplateFileName, defTypesDescriptionFileName, outputFileName);	
		break;
	case 2:
		
		break;
	case 3:

		break;
	case 4:

		break;
	}

	return 0;
}
