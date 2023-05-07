#include "wsjtx_lib.h"

/*
	To test the library, include "wsjtx_lib.h" from an application project
	and call wsjtx_libTest().
	
	Do not forget to add the library to Project Dependencies in Visual Studio.
*/

static int s_Test = 0;

int wsjtx_libTest()
{
	return ++s_Test;
}