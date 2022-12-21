/*
 * arguments.cpp
 *
 * Class for parsing program arguments - implementation.
 *
 * Project: Reader of news feed in format Atom & RSS with support of TLS
 * Author: Matus Remen (xremen01@stud.fit.vutbr.cz)
 * Date: 12.10.2022
 */

#include "../include/arguments.hpp"
#include <iostream>


Arguments::Arguments(int argc, char **argv){
	_url = std::string();
	_url_file = std::string();
	_cert_file = std::string();
	_certaddr = std::string();

	valid = parse_arguments(argc, argv);
}


Arguments::~Arguments(){}


bool Arguments::check_required(){
	// XOR
	if (_url.empty() != _url_file.empty())
		return true;
	return false;
}


bool Arguments::parse_arguments(int argc, char **argv){
	enum opt_id{
		FEEDFILE  = 'f',
		CERTFILE  = 'c',
		CERTADDR  = 'C',
		TIMESTAMP = 'T',
		AUTHORS   = 'a',
		ASS_URL   = 'u',
		HELP      = 'h',
	};
	static const struct option long_opts[] = {
		{"help", no_argument, nullptr, HELP},
		{nullptr, 0, nullptr, 0},
	};
	
	int opt;
	while ((opt = getopt_long(argc, argv, "f:c:C:Tauh", long_opts, nullptr)) != -1){
		switch (opt){
			case FEEDFILE:{
				_url_file = std::string(optarg);
				break;}
			case CERTFILE:{
				_cert_file = std::string(optarg);
				break;}
			case CERTADDR:{
				_certaddr = std::string(optarg);
				break;}
			case TIMESTAMP:{
				_opt_timestamp = true;
				break;}
			case AUTHORS:{
				_opt_author = true;
				break;}
			case ASS_URL:{
				_opt_ref = true;
				break;}
			case HELP:{
				print_usage();
				_opt_help = true;
				return true;}
			case '?':{
				if (optopt == 'f' || optopt == 'c' || optopt == 'C')
					std::cerr << "Missing argument for option '-" << optopt << "'." << std::endl;
				else
					std::cerr << "Unknown option '" << optopt << "'." << std::endl;
				return false;}
			default:{
				std::cerr << "Processing arguments failed." << std::endl;
				return false;}
		}
	}

	if (optind < argc){
		if (argc - optind > 1){
			std::cerr << "Extra arguments error." << std::endl;
			return false;
		}
		_url = std::string(argv[optind]);
	}

	return check_required();
}


void Arguments::print_usage(){
	std::cout << USAGE << std::endl;
}


std::string Arguments::get_url(){
	return _url;
}


std::string Arguments::get_url_file(){
	return _url_file;
}


std::string Arguments::get_cert_file(){
	return _cert_file;
}


std::string Arguments::get_certaddr(){
	return _certaddr;
}


bool Arguments::ok(){
	return valid;
}


bool Arguments::ts(){
	return _opt_timestamp;
}


bool Arguments::au(){
	return _opt_author;
}


bool Arguments::ref(){
	return _opt_ref;
}

