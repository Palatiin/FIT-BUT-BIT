/**
 * feedreader.cpp
 *
 * Project: Reader of news feed in format Atom & RSS with support of TLS
 * Author: Matus Remen (xremen01@stud.fit.vutbr.cz)
 * Date: 06.11.2022
 */


#include <fstream>
#include "../include/arguments.hpp"
#include "../include/client.hpp"
#include "../include/parser.hpp"


// Recognize type of url input (single url / file with urls)
// return list of urls
std::vector<std::string> get_urls(Arguments args){
	std::vector<std::string> url_list;
	std::string str = args.get_url();
	if (!str.empty()){
		url_list.push_back(str);
		return url_list;
	}

	std::ifstream file(args.get_url_file());
	if (!file.is_open()){
		std::cerr << "Can't open file with urls" << std::endl;
		return url_list;
	}

	while (getline(file, str)){
		if (str.empty())
			continue;
		else if (str[0] == '#')
			continue;
		url_list.push_back(str);
	}
	file.close();

	return url_list;
}


int main(int argc, char **argv){
	// parse input
	Arguments args = Arguments(argc, argv);
	if (!args.ok()){
		args.print_usage();
		return 1;
	}

	std::vector<std::string> urls = get_urls(args);

	// setup network client
	Client client = Client(args.get_cert_file(), args.get_certaddr());

	// loop through feed sources and print their messages
	for (std::string &url : urls){
		std::string response = client.request(url);

		if (!response.empty()){
			Parser parser = Parser(response, args.ts(), args.au(), args.ref());
			parser.parse_feed();
		}
	}

	return 0;
}

