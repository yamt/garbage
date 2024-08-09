#include <iostream>

int
main()
{
#if 1
        std::cout << "hello C++" << std::endl;
#endif
#if 0
        new char[1024*1024*16];
#endif
}

#if 0
extern "C" void abort_message(const char *fmt, ...)
{
	//fprintf(stderr, "my abort message\n");
}
#endif
