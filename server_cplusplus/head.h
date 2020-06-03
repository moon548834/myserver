#include <iostream>
#include <string>
#include <sys/socket.h>
#include <cstdlib>
#include <arpa/inet.h>
#include <unistd.h>
#include <regex>
#include <fstream>

#define PRINT 1;
#define COUT(x) std::cout << x << std::endl;
#define LOG(x) do { \
	std::ofstream f;		\
	f.open("./log"); \
	f << x << std::endl; \
	f.close();	\
}while(0)
