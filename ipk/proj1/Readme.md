# IPK 2021/2022 - First project - Lightweight Server
The goal of the project was to make a simple server
that is capable of responding to a requests about
system information.

## Prerequisities
* Linux Ubuntu 20.04 LTS
* g++

## Installation:
```$ make```

## Starting server:
```$ ./hinfosvc PORT_NUMBER```

## Stopping server:
Signal SIGINT - CTRL + C keyboard shortcut

## Supported paths:
* "/hostname"  - prints hostname
* "/cpu-info"  - prints cpu name, model, clock speed
* "/load"      - prints cpu load %

## Testing:
```curl http://localhost:12345/hostname``` -> merlin.fit.vutbr.cz
	
```curl http://localhost:12345/cpu-name``` -> Intel(R) Xeon(R) Silver 4214R CPU @ 2.40GHz
	
```curl http://localhost:12345/load``` -> 10%

## 
Author: Matúš Remeň (xremen01@stud.fit.vutbr.cz)<br/>
Date: 11.03.2022
