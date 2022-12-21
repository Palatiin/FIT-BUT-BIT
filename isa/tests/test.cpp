
#include <iostream>
#include <vector>
#include <string>
#include "../include/arguments.hpp"

void arg_test1(){
	int argc = 2;
	char *argv[] = {"feedreader", "https://www.test.com/feed"};
	Arguments args = Arguments(argc, argv);
	if (args.ok()){
		std::cout << "Test 01 OK" << std::endl;
	} else {
		std::cout << "Test 01 FAIL" << std::endl;
	}
}

void arg_test2(){
	int argc = 4;
	char *argv[] = {"feedreader", "https://www.test.com/feed", "-f", "feedfile.txt"};
	Arguments args = Arguments(argc, argv);
	if (args.ok()){
		std::cout << "Test 02 FAIL" << std::endl;
	} else {
		std::cout << "Test 02 OK"<< std::endl;
	}
}

void test_arguments(){
	arg_test1();
	arg_test2();
}


int main(){
	test_arguments();

	return 0;
}

