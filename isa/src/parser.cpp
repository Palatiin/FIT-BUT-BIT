/*
 * parser.cpp
 *
 * Class for Atom & RSS2.0 syndicate format parsers.
 *
 * Project: Reader of news feed in format Atom & RSS with support of TLS
 * Author: Matus Remen (xremen01@stud.fit.vutbr.cz)
 * Date: 05.11.2022
 */

#include "../include/parser.hpp"


Parser::Parser(std::string feed, bool _ts, bool _au, bool _ref){
	this->filter = 0;
	this->filter |= (_ts ? _TS_OPT : 0);
	this->filter |= (_au ? _AU_OPT : 0);
	this->filter |= (_ref ? _REF_OPT : 0);

	this->feed = feed;
	this->strip_feed();

	this->document = xmlParseDoc((const xmlChar *) this->feed.c_str());
	if (!this->document){
		xmlCleanupParser();
		this->document = nullptr;
		this->root = nullptr;
		return;
	}

	this->root = xmlDocGetRootElement(document);
	if (!this->root){
		xmlFreeDoc(this->document);
		xmlCleanupParser();
		this->document = nullptr;
		this->root = nullptr;
		return;
	}
}


Parser::~Parser(){
	if (document){
		xmlFreeDoc(this->document);
	}
	xmlCleanupParser();
}


void Parser::strip_feed(){
	const char *feed_c = this->feed.c_str();
	char *begin = nullptr;
	char *end = nullptr;

	for (const char *p = feed_c; *p; p++){
		if (!begin && *p == '<')
			begin = const_cast<char *>(p);
		else if (*p == '>')
			end = const_cast<char *>(p+1);
	}
	this->feed = std::string(begin, end);
}


void Parser::print_message(std::string title, std::string ts, std::string au, std::string ref, int idx){
	if (idx && this->filter && (!ts.empty() || !au.empty() || !ref.empty()))
		std::cout << "\n";
	std::cout << title << std::endl;
	if (this->filter & _TS_OPT && !ts.empty())
		std::cout << "Aktualizace: " << ts << std::endl;
	if (this->filter & _AU_OPT && !au.empty())
		std::cout << "Autor: " << au << std::endl;
	if (this->filter & _REF_OPT && !ref.empty())
		std::cout << "URL: " << ref << std::endl;
}


void Parser::parse_atom(){
	// title
	for (xmlNodePtr node = root->children; node; node = node->next)
		if (!xmlStrcasecmp(node->name, (xmlChar *) "title")){
			std::cout << "*** " << (char *) xmlNodeGetContent(node) << " ***" << std::endl;
			break;
		}

	// feed
	xmlNodePtr feed;
	for (feed = root->children; feed && xmlStrcasecmp(feed->name, (xmlChar *) "entry"); feed = feed->next);
	
	int idx = 0;
	for (feed = root->children; feed; feed = feed->next){
		if (xmlStrcasecmp(feed->name, (xmlChar *) "entry"))
			continue;

		// feed entries
		std::string title, timestamp, author, reference;
		for (xmlNodePtr entry = feed->children; entry; entry = entry->next){
			if (!xmlStrcasecmp(entry->name, (xmlChar *) "title"))
				title = std::string((char *) xmlNodeGetContent(entry));
			else if (!xmlStrcasecmp(entry->name, (xmlChar *) "updated"))
				timestamp = std::string((char *) xmlNodeGetContent(entry));
			else if (!xmlStrcasecmp(entry->name, (xmlChar *) "author")){
				xmlNodePtr author_node;
				for (
					author_node = entry->children;
					author_node && xmlStrcasecmp(author_node->name, (xmlChar *) "name");
					author_node = author_node->next
				);
				author = std::string((char *) xmlNodeGetContent(entry));
			} else if (!xmlStrcasecmp(entry->name, (xmlChar *) "link"))
				reference = std::string((char *) xmlGetProp(entry, (xmlChar *) "href"));
		}

		if (title.empty())
			continue;
		
		this->print_message(title, timestamp, author, reference, idx);
		idx++;
	}
}


void Parser::parse_rss2(){
	// title
	xmlNodePtr channel = nullptr;
	for (xmlNodePtr node = root->children; node; node = node->next){
		if (xmlStrcasecmp(node->name, (xmlChar *) "channel"))
			continue;
		for (channel = node->children; channel; channel = channel->next){
			if (!xmlStrcasecmp(channel->name, (xmlChar*) "title")){
				std::cout << "*** " << (char *) xmlNodeGetContent(channel) << " ***" << std::endl;
				channel = node;
				break;
			}
		}
		break;
	}

	// feed
	if (!channel) return;
	int idx = 0;
	for (xmlNodePtr item = channel->children; item; item = item->next){
		if (xmlStrcasecmp(item->name, (xmlChar *) "item"))
			continue;

		std::string title, timestamp, author, reference;
		for (xmlNodePtr node = item->children; node; node = node->next){
			if (!xmlStrcasecmp(node->name, (xmlChar *) "title"))
				title = std::string((char *) xmlNodeGetContent(node));
			else if (!xmlStrcasecmp(node->name, (xmlChar *) "pubDate"))
				timestamp = std::string((char *) xmlNodeGetContent(node));
			else if (!xmlStrcasecmp(node->name, (xmlChar *) "author"))
				author = std::string((char *) xmlNodeGetContent(node));
			else if (!xmlStrcasecmp(node->name, (xmlChar *) "link"))
				reference = std::string((char *) xmlNodeGetContent(node));
		}

		if (title.empty())
			continue;
		this->print_message(title, timestamp, author, reference, idx);
		idx++;
	}
}


void Parser::parse_feed(){
	if (!xmlStrcasecmp(root->name, (xmlChar *) "feed"))
		this->parse_atom();
	else if (!xmlStrcasecmp(root->name, (xmlChar *) "rss"))
		this->parse_rss2();
	else
		std::cerr << "Unknown feed format." << std::endl;
}

