/* 
 * IPK Project 2 - Variant ZETA - Packet Sniffer
 * Author: Matus Remen (xremen01@stud.fit.vutbr.cz)
 * Date: 24.04.2022
 */

#include <iostream>
#include <getopt.h>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <pcap.h>
#include <algorithm>
#include <exception>
#include <time.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>

struct Arguments {
	std::string interface;  // -i/--interface, specify interface to be sniffed
	int port = -1;          // -p, specify port to be sniffed
	int filter = 0;         // show only selected packets (tcp/udp/icmp, arp)
	int number = 1;         // number of packets to be caught
	bool valid = true;      // flag if parsing was successful
};

bool is_number(const std::string &str){
    // check if string is convertable to number
	for (const char &c : str){
        if (std::isdigit(c) == 0)
            return false;
    }
    return true;
}

Arguments parse_arguments(int argc, char **argv){
	Arguments args;
	enum opt_id{
		INTERFACE = 'i',
		PORT      = 'p',
		TCP       = 't',
		UDP       = 'u',
		ICMP      = 'c',
		ARP       = 'a',
		NUMBER    = 'n',
	};
	const struct option longopts[] = {
		{"interface", optional_argument, nullptr, INTERFACE},
		{"tcp",  no_argument, nullptr, TCP},
		{"udp",  no_argument, nullptr, UDP},
		{"icmp", no_argument, nullptr, ICMP},
		{"arp",  no_argument, nullptr, ARP},
		{nullptr, 0, nullptr, 0},
	};
	int arg;
	while ((arg = getopt_long(argc, argv, "i::p:tun:", longopts, nullptr)) != -1){
		switch (arg){
			case INTERFACE:{
				if (optind >= argc){
					break;
				}
				if (*argv[optind] == '-')
					break;
				args.interface = std::string{argv[optind]};
				break;}
			case PORT:{
				std::stringstream port_ss(optarg);
				if (!is_number(optarg) || !(port_ss >> args.port))
					args.valid = false;
				break;}
			case TCP:{
				args.filter |= 0b1;
				break;}
			case UDP:{
				args.filter |= 0b10;
				break;}
			case ARP:{
				args.filter |= 0b100;
				break;}
			case ICMP:{
				args.filter |= 0b1000;
				break;}
			case NUMBER:{
				std::stringstream num_ss(optarg);
				if (!is_number(optarg) || !(num_ss >> args.number))
					args.valid = false;
				if (args.number < 1 || args.number > 65535)
					args.valid = false;
				break;}
			default:{
				args.valid = false;
				break;}
		}
	}
	
	return args;
}

std::vector<std::string> list_available_devices(){
	// get list of available devices for sniffing
	
	std::vector<std::string> dev_list;
	pcap_if_t *alldevsp;
	char errbuf[PCAP_ERRBUF_SIZE];
	if (pcap_findalldevs(&alldevsp, errbuf)){
		std::cout << "list_available_devices: " << errbuf << std::endl;
		return dev_list;
	}
	// get interface names into nicer structure
	while (alldevsp){
		if (alldevsp->name != nullptr)
			dev_list.push_back(std::string{alldevsp->name});
		alldevsp = alldevsp->next;
	}

	return dev_list;
}

// Exceptions
struct LookupNetException : public std::exception {
	const char * what () const throw (){
		return "Couldn't get netmask for selected device";
	}
};

struct OpenLiveException : public std::exception {
	const char * what () const throw (){
		return "Couldn't open device";
	}
};

struct CompileFilterException : public std::exception {
	const char * what () const throw (){
		return "Couldn't parse filter";
	}
};

struct SetFilterException : public std::exception {
	const char * what () const throw (){
		return "Couldn't install filter";
	}
};

struct IPHeaderLengthException : public std::exception {
	const char * what () const throw (){
		return "Invalid IP header length";
	}
};

struct TCPHeaderLengthException : public std::exception {
	const char * what () const throw (){
		return "Invalid TCP header length";
	}
};

struct PcapLoopException : public std::exception {
	const char * what () const throw (){
		return "Invalid TCP header length";
	}
};
// /Exceptions

void print_packet_data(const u_char *data, bpf_u_int32 length){
	// http://www.dmulholl.com/lets-build/a-hexdump-utility.html
    int line_counter = 0x0000;
    for(bpf_u_int32 i = 0; i < length; i+= 0x10) {
        printf("0x%04x: ", line_counter);
        line_counter += 0x0010;

        for (bpf_u_int32 j = 0; j < 0x10; j++) {
            if (j > 0 && j % 8 == 0)
                printf(" ");
            if (i+j < length)
                printf(" %02x", data[i+j]);
            else
                printf("   ");
        }

        printf("  ");

        for (bpf_u_int32 j = 0; j < 0x10 && i+j < length; j++) {
            if (j > 0 && j % 8 == 0)
				printf(" ");
			if (isprint(data[i+j]))
                printf("%c", data[i+j]);
            else
                printf(".");
        }
        printf("\n");
    }
    printf("\n");
}

void print_tcp(const u_char *packet, int ip_header_len, bpf_u_int32 len){
	// print src/dst port in tcp protocol and packet hexdump
	
	struct tcphdr *tcp = (struct tcphdr *)(packet + sizeof(struct ether_header) + ip_header_len);
	std::cout << "src port: " << ntohs(tcp->th_sport) << std::endl;
	std::cout << "dst port: " << ntohs(tcp->th_dport) << std::endl;
	std::cout << std::endl;

	print_packet_data(packet, len);
}

void print_udp(const u_char *packet, int ip_header_len, bpf_u_int32 len){
	// print src/dst port in udp protocol and packet hexdump
	
	struct udphdr *udp = (struct udphdr *)(packet + sizeof(ether_header) + ip_header_len);
	int headers_len = sizeof(struct ether_header) + ip_header_len + udp->uh_ulen;

	std::cout << "src port: " << ntohs(udp->uh_sport) << std::endl;
	std::cout << "dst port: " << ntohs(udp->uh_dport) << std::endl;
	std::cout << std::endl;

	print_packet_data(packet, len);
}

void print_icmp(const u_char *packet, int ip_header_len, bpf_u_int32 len){
	// print packet hexdump of protocol icmp
	
	std::cout << std::endl;
	print_packet_data(packet, len);
}

// callback function
void process_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet){
	// during implementation of this function this link was very helpful:
	// https://elixir.bootlin.com/uclibc-ng/v1.0.20/source/include/netinet
	
	// get timestamp from packet header
	char buffer[128] = {0, };
	char time[64] = {0, };
	struct tm * tm = localtime(&header->ts.tv_sec);
	strftime(time, sizeof(time), "%FT%T", tm);
	snprintf(buffer, sizeof(buffer), "%s.%03ld", time, header->ts.tv_usec / 1000);
	strftime(time, sizeof(time), "%z", tm);
	std::cout << "timestamp: " << buffer << " " << time << std::endl;

	// get src and dst MAC address and frame length from ethernet header
	struct ether_header *ethernet = (struct ether_header*)(packet);
	char src_mac[20] = {0, };
	char dst_mac[20] = {0, };
	snprintf(
		src_mac,
		19,
		"%02x:%02x:%02x:%02x:%02x:%02x",
		ethernet->ether_shost[0],
		ethernet->ether_shost[1],
		ethernet->ether_shost[2],
		ethernet->ether_shost[3],
		ethernet->ether_shost[4],
		ethernet->ether_shost[5]
	);
	snprintf(
		dst_mac,
		19,
		"%02x:%02x:%02x:%02x:%02x:%02x",
		ethernet->ether_dhost[0],
		ethernet->ether_dhost[1],
		ethernet->ether_dhost[2],
		ethernet->ether_dhost[3],
		ethernet->ether_dhost[4],
		ethernet->ether_dhost[5]
	);
	std::cout << "src MAC: " << src_mac << std::endl;
	std::cout << "dst MAC: " << dst_mac << std::endl;
	std::cout << "frame length: " << header->len << " bytes" << std::endl;

	// get type of packet (IPv4/IPv6/ARP)
	if (ntohs(ethernet->ether_type) == ETHERTYPE_IPV6){
		// https://stackoverflow.com/questions/66784119/getting-npcap-ipv6-source-and-destination-addresses
		struct ip6_hdr *ip6 = (struct ip6_hdr *)(packet + sizeof(struct ether_header));
		char saddr[INET6_ADDRSTRLEN] = {0, };
		char daddr[INET6_ADDRSTRLEN] = {0, };
		inet_ntop(AF_INET6, &(ip6->ip6_src), saddr, INET6_ADDRSTRLEN);
		inet_ntop(AF_INET6, &(ip6->ip6_dst), daddr, INET6_ADDRSTRLEN);
		std::cout << "src IP: " << saddr << std::endl;
		std::cout << "dst IP: " << daddr << std::endl;

		// print protocol data
		switch (ip6->ip6_ctlun.ip6_un1.ip6_un1_nxt){
			case IPPROTO_ICMP:
				print_icmp(packet, 40, header->len);
				break;
			case IPPROTO_TCP:
				print_tcp(packet, 40, header->len);
				break;
			case IPPROTO_UDP:
				print_udp(packet, 40, header->len);
				break;
		}
	}
	else if (ntohs(ethernet->ether_type) == ETHERTYPE_ARP){
		std::cout << std::endl;
		print_packet_data(packet, header->len);
	}
	else {	// IPv4
		struct ip* ip = (struct ip *)(packet + sizeof(struct ether_header));
		std::cout << "src IP: " << inet_ntoa(ip->ip_src) << std::endl;
		std::cout << "dst IP: " << inet_ntoa(ip->ip_dst) << std::endl;

		// print protocol data
		switch (ip->ip_p){
			case IPPROTO_ICMP:
				print_icmp(packet, ip->ip_hl*4, header->len);
				break;
			case IPPROTO_TCP:
				print_tcp(packet, ip->ip_hl*4, header->len);
				break;
			case IPPROTO_UDP:
				print_udp(packet, ip->ip_hl*4, header->len);
				break;
		}

	}
}


class Sniffer{
/* This class initializes sniffer, prepares filter expression, opens interface
 * for live capture and sniffs packet traffic on that interface.
 *
 * Source: https://www.tcpdump.org/pcap.html
 * Copyright 2002 Tim Carstens. All rights reserved. 
 * Redistribution and use, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 *  1. Redistribution must retain the above copyright notice and this list of conditions. 
 *  2. The name of Tim Carstens may not be used to endorse or promote products derived
 *     from this document without specific prior written permission.
 */
	Arguments args;
	pcap_t *handle;
	char errbuf[PCAP_ERRBUF_SIZE];
	std::string filter_expression;
	struct bpf_program filter;
	bpf_u_int32 mask;
	bpf_u_int32 net;

public:
	Sniffer(const Arguments args) : args(args){
		// initialize: find interface, open for live capture, setup filter
		if (pcap_lookupnet(args.interface.c_str(), &(this->net), &(this->mask), this->errbuf) == -1){
			throw LookupNetException();
		}
		this->open_live();
		this->setup_filter();
	}

	~Sniffer(){
		// close live capturing on interface
		pcap_close(this->handle);
	}

	void sniff(){
		// start sniffer
		if (pcap_loop(this->handle, this->args.number, (pcap_handler)(*process_packet), nullptr) < 0)
			throw PcapLoopException();
	}

private:
	void open_live(){
		// open for live capture
		this->handle = pcap_open_live(args.interface.c_str(), BUFSIZ, 1, 1000, this->errbuf);
		if (this->handle == nullptr){
			throw OpenLiveException();
		}
	}

	void build_filter_expression(){
		// prepare filter expression to be installed on sniffer
		this->filter_expression = "";
		if (this->args.filter & 1){
			this->filter_expression += "tcp";
		}
		if (this->args.filter & 2){
			if (this->filter_expression != "")
				this->filter_expression += " or ";
			this->filter_expression += "udp";
		}
		if (this->args.filter & 4){
			if (this->filter_expression != "")
				this->filter_expression += " or ";
			this->filter_expression += "arp";
		}
		if (this->args.filter & 8){
			if (this->filter_expression != "")
				this->filter_expression += " or ";
			this->filter_expression += "icmp";
		}

		if (this->args.port > -1){
			if (this->filter_expression != "")
				this->filter_expression = "(" + this->filter_expression + ") and ";
			this->filter_expression += "port " + std::to_string(this->args.port);
		}
	}

	void setup_filter(){
		// compile and install filter expression
		this->build_filter_expression();
		if (this->filter_expression == "")
			return;

		if (pcap_compile(this->handle, &(this->filter), this->filter_expression.c_str(), 0, this->net) == -1){
			throw CompileFilterException();
		}
		if (pcap_setfilter(this->handle, &(this->filter)) == -1){
			throw SetFilterException();
		}
	}
};


int main(int argc, char **argv){
	// parse arguments
	Arguments args = parse_arguments(argc, argv);	
	if (!args.valid){
		std::cerr << "main: Invalid arguments" << std::endl;
		return 1;
	}

	// get available devices and print them if arguments say so
	std::vector<std::string> dev_list;
	dev_list = list_available_devices();

	if (args.interface == ""){
		for (std::string device : dev_list){
			std::cout << device << std::endl;
		}
		return 0;
	}
	else if (std::find(dev_list.begin(), dev_list.end(), args.interface) == dev_list.end()){
		std::cout << "main: Unknown interface" << std::endl;
		return 1;
	}

	// lets sniff some packets :P
	Sniffer sniffer(args);
	sniffer.sniff();

	return 0;
}
