/*
 * parser.hpp
 *
 * Class for Atom & RSS2.0 syndicate format parsers.
 *
 * Project: Reader of news feed in format Atom & RSS with support of TLS
 * Author: Matus Remen (xremen01@stud.fit.vutbr.cz)
 * Date: 05.11.2022
 */


#ifndef _PARSER_HPP
#define _PARSER_HPP


#define _TS_OPT 1
#define _AU_OPT 2
#define _REF_OPT 4

#define _ATOM 1
#define _RSS2 2

#include <string>
#include <iostream>
#include <libxml/parser.h>


/*
 * Class for parsing messages from feeds.
 * Supported formats: Atom, RSS 2.0
 *
 * Provides option to display message timestamps, author names and references.
 */
class Parser {
private:
	std::string feed;    // repsponse body - feed
	xmlDocPtr document;  // feed xml document object
	xmlNodePtr root;     // root node of xml document
	char filter;         // filter with display options

	// strip unwanted characters outside xml document
	// 'https://www.fit.vut.cz/fit/news-rss/' contained unwanted characters outside xml which caused
	// xml document setup lead to failure
	void strip_feed();

	// print structured message considering display filter
	void print_message(std::string title, std::string ts, std::string au, std::string ref, int idx);
	// method for parsing feed in Atom format
	void parse_atom();
	// method for parsing feed in RSS 2.0 format
	void parse_rss2();

public:
	Parser(std::string feed, bool _ts, bool _au, bool _ref);
	~Parser();

	// Method for parsing feed, recognizes format and calls respective private parsing method
	// for the format.
	// prints feed messages
	void parse_feed();
};

#endif

