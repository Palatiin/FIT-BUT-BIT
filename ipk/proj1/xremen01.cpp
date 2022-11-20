/*
 *	xremen01.cpp
 *	IPK 2021/2022 - First Project - Lightweight Server
 *	Matus Remen (xremen01@stud.fit.vutbr.cz)
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <signal.h>
#include <map>
#include <fstream>
#include <sstream>
#include <regex>


int sockfd = -1;
int connection = -1;


bool is_number(const std::string &str){
	for (const char &c : str){
		if (std::isdigit(c) == 0)
			return false;
	}
	return true;
}


// handler for SIGINT (Ctrl + C)
void sig_handler(int signal){
	std::cout << "\nShutting down server" << std::endl;
	if (connection >= 0){
		std::cout << "Closing connection" << std::endl;
		close(connection);
	}
	if (sockfd >= 0){
		std::cout << "Closing socket file descriptor" << std::endl;
		close(sockfd);
	}
	exit(0);
}


// parse HTTP header - method and path
std::map<std::string, std::string> parse_request(const char *msg){
	std::map<std::string, std::string> parsed_request;
	const char *head = msg;
    const char *tail = msg;

    while (*tail != '\r' && *tail != ' ') ++tail;
    parsed_request["method"] = std::string(head, tail);

    while (*tail != '\r' && *tail == ' ') ++tail;
    head = tail;
    
	while (*tail != '\r' && *tail != ' ') ++tail;
    parsed_request["path"] = std::string(head, tail);
    
	return parsed_request;
}


void get_cpu_name(std::string &cpu_name, int *code){
	std::regex re_cpu_name("model\\sname\\s*:\\s*(.+)");
	std::string line;
	std::ifstream file("/proc/cpuinfo");
	if (file.good()){	
		std::smatch match;
		while (std::getline(file, line)){
			if (std::regex_search(line, match, re_cpu_name)){
				cpu_name = match[1].str();
				break;
			}
		}
		file.close();
	} else {
		*code = 500;
	}
}


// split string by spaces
std::vector<std::string> split_string(const std::string &str){
	std::vector<std::string> result;
	std::istringstream iss(str);
	for (std::string s; iss >> s; )
		result.push_back(s);

	return result;
}


void get_cpu_load(std::string &load, int *code){
	std::vector<std::string> cpu_data1;
	std::vector<std::string> cpu_data2;
	
	// get cpu data
	std::ifstream file("/proc/stat");
	if (file.good()){
		std::string line;
		std::getline(file, line);
		cpu_data1 = split_string(line);
		file.close();
	} else {
		*code = 500;
		return;
	}
	
	sleep(1);
	
	std::ifstream file2("/proc/stat");
	if (file.good()){
		std::string line;
		std::getline(file2, line);
		cpu_data2 = split_string(line);
		file2.close();
	} else {
		*code = 500;
		return;
	}

	// calculate cpu load %
	long int total_jiffies1 = 0;
	long int total_jiffies2 = 0;
	long int work_jiffies1 = 0;
	long int work_jiffies2 = 0;
	for (int i = 1; i < cpu_data1.size(); ++i)
		total_jiffies1 += stol(cpu_data1[i]);
	for (int i = 1; i < cpu_data2.size(); ++i)
		total_jiffies2 += stol(cpu_data2[i]);
	work_jiffies1 = stol(cpu_data1[1]) + stol(cpu_data1[2]) + stol(cpu_data1[3]);
	work_jiffies2 = stol(cpu_data2[1]) + stol(cpu_data2[2]) + stol(cpu_data2[3]);

	long double work_diff = work_jiffies2 - work_jiffies1;
	long double total_diff = total_jiffies2 - total_jiffies1;
	long double cpu_load = work_diff / total_diff * 100.0;

	load = std::to_string(cpu_load) + "%";
}


// processing request and sending response
void response(const std::string &request_buffer){
	int status_code = 200;
	std::string content;
	std::map<int, std::string> status_code_repr;
	std::map<std::string, std::string> request;
	
	status_code_repr[200] = "OK";
	status_code_repr[400] = "Bad Request";
	status_code_repr[404] = "Not Found";
	status_code_repr[500] = "Internal Server Error";

	request = parse_request(request_buffer.c_str());

	// method check
	if (request["method"] != "GET"){
		std::cout << "Request method not supported (" << request["method"] << ")" << std::endl;
		status_code = 400;
	}

	// path check
	if (status_code == 200){
		if (request["path"] == "/hostname"){
			char hostname[128] = {0,};
			gethostname(hostname, 128);
			content = std::string(hostname);
		}
		else if (request["path"] == "/cpu-name"){
			get_cpu_name(content, &status_code);
		}
		else if (request["path"] == "/load"){
			get_cpu_load(content, &status_code);
		}
		else {
			std::cout << "Unknown path" << std::endl;
			status_code = 404;
		}
	}
	

	// form HTTP headers for response
	int content_length = std::string(content).size();
	std::string headers =
		"HTTP/1.1 " + std::to_string(status_code) + status_code_repr[status_code] + "\r\n" 
		"Connection: close\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: " + std::to_string(content_length) + "\r\n\r\n";

	// send headers	
	if (send(connection, headers.c_str(), headers.size(), 0) == -1){
		std::cout << "Err: headers not sent" << std::endl;
	}
	// send content
	if (send(connection, content.c_str(), content_length, 0) == -1){
		std::cout << "Err: content not sent" << std::endl;
	}
}

// ===========================================================================

int main(int argc, char *argv[]){
	if (argc != 2){
		std::cout << "Missing argument with port number" << std::endl;
		return 1;
	}
	if (!is_number(argv[1])){
		std::cout << "Argument of port number is not integer" << std::endl;
		return 2;
	}
	int port = std::stoi(argv[1]);
	if (0 > port || port > 65535){
		std::cout << "Port number is not valid. Must be a number in range <0, 65535>" << std::endl;
		return 3;
	}

	// bind signal handler
	struct sigaction sig_int_handler;
	sig_int_handler.sa_handler = sig_handler;
	sigemptyset(&sig_int_handler.sa_mask);
	sig_int_handler.sa_flags = 0;
	sigaction(SIGINT, &sig_int_handler, NULL);
	
	// create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1){
		std::cout << "Failed to create socket" << std::endl;
	}
	std::cout << "Socket created (" << sockfd << ")" << std::endl;

	// bind socket to local port
	sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(port);
	auto addrlen = sizeof(sockaddr);
	if (bind(sockfd, (struct sockaddr*) &sockaddr, addrlen) < 0){
		std::cout << "Failed to bind socket to port " << port << std::endl; 
		return 4;
	}

	// max 5 requests in queue
	if (listen(sockfd, 5) < 0){
		std::cout << "Failed to begin listening on socket" << std::endl;
	}

	// main loop - reading client's requests
	while (1){
		connection = accept(sockfd, (struct sockaddr*) &sockaddr, (socklen_t*) &addrlen);
		if (connection < 0){
			std::cout << "Failed to create connection" << std::endl;
		}

		char request_buff[512];

		auto request = read(connection, request_buff, 512);
		response(request_buff);
		close(connection);
	}
	close(connection);
	close(sockfd);

	return 0;
}
