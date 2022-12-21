/*
 * arguments.hpp
 *
 * Class for parsing program arguments utilizing getopt.h library.
 *
 * Project: Reader of news feed in format Atom & RSS with support of TLS
 * Author: Matus Remen (xremen01@stud.fit.vutbr.cz)
 * Date: 12.10.2022
 */

#ifndef _ARGUMENTS_HPP
#define _ARGUMENTS_HPP

#include <string>
#include <getopt.h>


/**
 * Definition of object which holds program setup options.
 * On initialization argument parsing is preformed.
 * If arguments are invalid, 'valid' attribute is set to 'false'.
 */
class Arguments {
private:
	bool valid;  // flag representing validity of program arguments

	// mandatory args - can not be both set simultaneously
	std::string _url;        // feed source url
	std::string _url_file;   // path to file with feed source urls

	// optional args
	std::string _cert_file;  // certificate file
	std::string _certaddr;   // file for server certificate validation

	bool _opt_timestamp = false;  // option to print timestamps for feed messages
	bool _opt_author = false;     // option to print authors of messages
	bool _opt_ref = false;        // option to print references for feed messages
	bool _opt_help = false;       // option to print program usage

	const std::string USAGE =
		"Usage: feedreader <URL | -f <feedfile>> [-c <certfile>] [-C <certaddr>] [-T] [-a] [-u]\n"
		"\n"
		"Args:\n"
		"    URL           - target feed URL\n"
		"    -f <feedfile> - path to file with feed URLs\n"
		"\n"
		"    -c <certfile> - path to file with certificates\n"
		"    -C <certaddr> - directory with certificates\n"
		"    -T            - print timestamps\n"
		"    -a            - print authors\n"
		"    -u            - print associated URL\n";


	// check if required arguments were passed to program
	bool check_required();
	// method for parsing program arguments, sets object attributes
	bool parse_arguments(int argc, char **argv);

public:
	Arguments(int argc, char **argv);
	~Arguments();

	// print program usage --help
	void print_usage();

	// getter methods for object attributes
	// return source url
	std::string get_url();
	// return file with source urls
	std::string get_url_file();
	
	// return certificate file
	std::string get_cert_file();
	// return certificate validation file
	std::string get_certaddr();

	// check valid flag
	bool ok();
	// check timestamp flag
	bool ts();
	// check author flag
	bool au();
	// check reference flag
	bool ref();
};

#endif
