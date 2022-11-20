**IPK Projekt 2 - Varianta ZETA - Sniffer paketov**<br/>
**Autor: Matúš Remeň (xremen01@stud.fit.vutbr.cz)**

## Prerekvizity
- Linux OS
- make
- Super User práva

Implementované na: `Ubuntu 20.04.4 LTS`<br/>
Preložené s: `gcc version 9.4.0 (Ubuntu 9.4.0-1ubuntu1~20.04.1)`

## Inštalácia
1. `tar -xvf xremen01.tar` (*)
2. `make`<br/>
alebo<br/>
`g++ -std=c++17 packet_sniffer.cpp -o ipk-sniffer -lpcap`

## Spúšťanie snifferu
`sudo ./ipk-sniffer [-i rozhraní | --interface rozhraní] {-p port} {[--tcp|-t] [--udp|-u] [--arp] [--icmp] } {-n num}`

- -i/--interface - rozhranie, ku ktorému sa bude naslúchať
- -p - filtrovanie packetov na danom rozhraní podla portu
- -t/--tcp - filtrovanie TCP packetov
- -u/--udp - filtrovanie UDP packetov
- --arp - filtrovanie ARP rámcov
- -n num - počet packetov, kolko má byť zachytených

Argumenty môžu byť v ľubovolnom poradí. Hranaté zátvorky [ ] označujú povinné argumenty
a kučeravé { } volitelné (keď nie su použité, použijú sa defaultné hodnoty)

## Výstup
### Popis
```
 timestamp: čas (RFC3339)
 src MAC: MAC adresa s : jako oddělovačem
 dst MAC: MAC adresa s : jako oddělovačem
 frame length: délka
 src IP: pokud je tak IP adresa (podpora v4 ale i v6 dle RFC5952)
 dst IP: pokud je tak IP adresa (podpora v4 ale i v6 dle RFC5952)
 src port: pokud je tak portové číslo
 dst port: pokud je tak portové číslo
 
 offset_vypsaných_bajtů:  výpis_bajtů_hexa výpis_bajtů_ASCII
```
### Vzor
```
timestamp: 2022-04-24T23:21:21.970 +0200
src MAC: 3c:2c:30:c8:d3:68
dst MAC: 01:00:5e:00:00:fb
frame length: 81 bytes
src IP: ddd.ddd.ddd.ddd
dst IP: ddd.ddd.ddd.ddd
src port: 5353
dst port: 5353

0x0000:  01 00 5e 00 00 fb 3c 2c  30 c8 d3 68 08 00 45 00  ..^...<, 0..h..E.
0x0010:  00 43 ba 20 00 00 01 11  ca 40 93 e5 c0 68 e0 00  .C. .... .@...h..
0x0020:  00 fb 14 e9 14 e9 00 2f  1a 55 00 00 00 00 00 01  ......./ .U......
0x0030:  00 00 00 00 00 00 0f 42  52 57 32 38 35 36 35 41  .......B RW28565A
0x0040:  36 39 35 30 44 30 05 6c  6f 63 61 6c 00 00 01 00  6950D0.l ocal....
0x0050:  01                                                .
```

### Validácia výsledkov
S pomocou aplikácie **Wireshark**.

### Zdroje
- https://www.tcpdump.org/pcap.html
- https://elixir.bootlin.com/uclibc-ng/v1.0.20/source/include/netinet
- http://www.dmulholl.com/lets-build/a-hexdump-utility.html
- https://stackoverflow.com/questions/66784119/getting-npcap-ipv6-source-and-destination-addresses
