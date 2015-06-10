#include <iostream>
#include <typeinfo>

using namespace std;

int main(int argc, char const *argv[])
{
	#ifndef NULL3
   	#define NULL3 //10
   		//cout << NULL3<< '\n';
		std::cout << typeid(NULL3).name() << '\n';
	#endif
   		cout << "34"<< '\n';
   		return 0;
}
