/*
 * client.hpp
 *
 * Client for fetching data from feeds.
 *
 * Project: Reader of news feed in format Atom & RSS with support of TLS
 * Author: Matus Remen (xremen01@stud.fit.vutbr.cz)
 * Date: 12.10.2022
 */

#ifndef _CLIENT_HPP
#define _CLIENT_HPP

#include <map>
#include <regex>
#include <string>
#include <iostream>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


// size of buffer where response from server is read
#define BUFFER_SIZE 8192

// url structure, holds parsed url properties
struct url {
	bool valid;
	std::string protocol;
	std::string authority;
	std::string path;
	std::string port;
};


/*
 * Client object wraps openssl library for our purposes of connection to feed sources
 * and return if it's contents with support of TLS.
 */
class Client {
private:
	std::string certfile;
	std::string certaddr;

	BIO *bio;      // Basic Input Output API, used for sending and receiving messages via sockets - network
	SSL_CTX *ctx;  // SSL object configurator, factory

	std::map<std::string, std::string> PORT_MAP;  // mapping of protocols to ports

	// clean resources
	void cleanup();

	// method for parsing url string into url structure for easier manipulation
	struct url parse_url(std::string url);
	// method for setting up SSL_CTX by loading certificates based on protocol and user setup
	bool verify_certificate(std::string protocol, std::string authority);
	// socket and SSL setup
	bool socket_init(struct url _url);
	
	// method for parsing server response, for successful responses (code 200) returns it's body
	std::string get_response_body(std::string response);
public:
	Client(std::string certfile, std::string certaddr);
	~Client();

	// setup connection to server and send request, returns response body
	std::string request(std::string url);
};

#endif

