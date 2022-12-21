/*
 * client.cpp
 *
 * Client for fetching data from feeds.
 *
 * Project: Reader of news feed in format Atom & RSS with support of TLS
 * Author: Matus Remen (xremen01@stud.fit.vutbr.cz)
 * Date: 12.10.2022
 */


#include "../include/client.hpp"


Client::Client(std::string certfile, std::string certaddr){
	PORT_MAP["http"] = "80";
	PORT_MAP["https"] = "443";
	
	SSL_library_init();
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	OpenSSL_add_all_algorithms();

	this->certfile = certfile;
	this->certaddr = certaddr;

	this->bio = nullptr;
	this->ctx = nullptr;
}


Client::~Client(){
	cleanup();
}


void Client::cleanup(){
	if (this->bio){
		BIO_free_all(this->bio);
		this->bio = nullptr;
	}
	if (this->ctx){
		SSL_CTX_free(this->ctx);
		this->ctx = nullptr;
	}
}


struct url Client::parse_url(std::string url){
	struct url parsed_url = {false, std::string(), std::string(), std::string(), std::string()};
	static std::regex re_url(R"(^(https?)://([^/?#:]+)(:[0-9]+)?(.*)$)");
	std::smatch match;

	if (!std::regex_match(url, match, re_url))
		return parsed_url;
	parsed_url.protocol = std::string(match[1]);
	parsed_url.authority = std::string(match[2]);
	parsed_url.path = std::string(match[4]);
	std::string port = std::string(match[3]);
	if (port.empty()){
		parsed_url.port = PORT_MAP[parsed_url.protocol];
		parsed_url.authority += ":" + std::string(parsed_url.port);
	}
	parsed_url.valid = true;

	return parsed_url;
}


bool Client::verify_certificate(std::string protocol, std::string authority){
	if (protocol != "https"){
		this->bio = BIO_new_connect(authority.c_str());
		return true;
	}

	int verification = 0;
	this->ctx = SSL_CTX_new(SSLv23_client_method());
	if (certfile.empty() && certaddr.empty()){
		verification = SSL_CTX_set_default_verify_paths(this->ctx);
	}
	else if (!certfile.empty() && !certaddr.empty()){
		verification = SSL_CTX_load_verify_locations(this->ctx, certfile.c_str(), certaddr.c_str());
	}
	else if (!certfile.empty()){
		verification = SSL_CTX_load_verify_locations(this->ctx, certfile.c_str(), nullptr);
	}
	else {
		verification = SSL_CTX_load_verify_locations(this->ctx, nullptr, certaddr.c_str());
	}

	if (!verification){
		return false;
	}

	this->bio = BIO_new_ssl_connect(this->ctx);

	return true;
}


bool Client::socket_init(struct url _url){
	if (!verify_certificate(_url.protocol, _url.authority)){
		std::cerr << "Error: Verification of certificates failed." << std::endl;
		return false;
	}
	if (!this->bio){
		std::cerr << "Error: Socket setup failure." << std::endl;
		return false;
	}
	
	SSL *ssl = nullptr;
	if (_url.protocol == "https"){
		BIO_get_ssl(this->bio, &ssl);
		SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
		BIO_set_conn_hostname(this->bio, _url.authority.c_str());
	}

	if (BIO_do_connect(this->bio) <= 0){
		std::cerr << "Error: Handshake failure." << std::endl;
		return false;
	}

	if (ssl && SSL_get_verify_result(ssl) != X509_V_OK){
		std::cerr << "Chyba: nepodařilo se ověřit platnost certifikátu serveru " << _url.authority << std::endl;
		return false;
	}
	return true;
}


std::string Client::get_response_body(std::string response){
	std::map<std::string, std::string> parsed_response;
	const char *resp_cstr = response.c_str();
    const char *head = resp_cstr;
    const char *tail = resp_cstr;

    while (*tail != '\r' && *tail != ' ') ++tail;
    parsed_response["http_v"] = std::string(head, tail);

    while (*tail != '\r' && *tail == ' ') ++tail;
    head = tail;
        
    while (*tail != '\r' && *tail != ' ') ++tail;
    parsed_response["status_code"] = std::string(head, tail);

	if (parsed_response["status_code"][0] != '2'){
		std::cerr << "Return code: " << parsed_response["status_code"] << std::endl;
		return std::string();
	}
    
	size_t body_start = response.find("\r\n\r\n");
	body_start = body_start == std::string::npos ? response.find("\n\n") + 2 : body_start + 4;

    return response.substr(body_start);
}


std::string Client::request(std::string url){
	static unsigned int request_number = 0;

	// 1. connection init
	struct url _url = parse_url(url);

	if (request_number++) std::cout << "\n";
	if (!_url.valid){
		std::cerr << "Invalid url '" << url << "'." << std::endl;
		cleanup();
		return std::string();
	}
	
	if (!socket_init(_url)){
		cleanup();
		return std::string();
	}

	// 2. send request
	std::string http_request(
		"GET " + _url.path + " HTTP/1.1\r\n"
		"Host: " + _url.authority + "\r\n"
		"Connection: close\r\n"
		"User-Agent: isa-project/feedreader (FIT-BUT)\r\n\r\n"
	);

	bool sent = false;
	do {
		if (BIO_write(this->bio, http_request.c_str(), static_cast<int>(http_request.size())))
			sent = true;
	} while (BIO_should_retry(this->bio) && !sent);

	if (!sent){
		cleanup();
		return std::string();
	}

	// 3. get response
	std::string http_response;
	char buffer[BUFFER_SIZE] = {0,};
	int read = 0;
	do {
		bool success = false;
		do {
			if ((read = BIO_read(this->bio, buffer, BUFFER_SIZE - 1)) > 0){
				buffer[read] = 0;
				http_response += buffer;
			}
			if (read >= 0)
				success = true;
		} while (BIO_should_retry(this->bio) && !success);

		if (!success){
			std::cerr << "Error while reading response." << std::endl;
			cleanup();
			return std::string();
		}
	} while (read);
	
	cleanup();
	return get_response_body(http_response);
}

