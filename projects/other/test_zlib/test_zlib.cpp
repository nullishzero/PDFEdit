// How to add zlib
//- download zlib - http://www.zlib.net/
//- compile DLL, in debug mode it should produce zlib1d.dll & zlib1d.lib
//- put the zlib1d.lib to projects\output\libs\
//- add zlib1d.lib to Linker->Input in project Properties 
//- put the dll to known directory (e.g. from which it is executed)

#include <zlib.h>

int main(int , char* [])
{
	z_stream z;
	return 0;
}

