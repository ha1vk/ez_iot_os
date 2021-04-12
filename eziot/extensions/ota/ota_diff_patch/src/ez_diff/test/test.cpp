// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\ez_diff.h"
#include <stdio.h>

int _tmain(int argc, _TCHAR* argv[])
{
	if(argc < 4)
	{
		printf("usage: oldfile newfile patchfile\n");
		return -1;
	}
	
	printf("old:%s\n", argv[1]);
	printf("new:%s\n", argv[2]);
	printf("patch:%s\n", argv[3]);
	ez_diff(argv[1], argv[2], argv[3], 1);

	return 0;
}

