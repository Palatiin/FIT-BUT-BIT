/* proj2.h
 * autor: Matúš Remeň, FIT
 * prelozene: gcc version 9.3.0 (Ubuntu 9.3.0-17ubuntu1~20.04)
 * popis: Hlavickovy subor pre zdrojovy subor proj2.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>

#define LOCKED   0
#define UNLOCKED 1

#define ELF_LIMIT      1000
#define REINDEER_LIMIT 20
#define TIME_LIMIT     1000

typedef struct {
	int NE;			// number of elves
	int NR;			// number of reindeers
	int TE;			// time of elve's solo work
	int TR;			// time of reindeer's vacation
} args_t;

int init_resources();
void free_resources();
void generate_elves(int NE, int TE);
void generate_reindeer(int NR, int TR);
unsigned rand_time_elf(unsigned max_time);
unsigned rand_time_reindeer(unsigned max_time);
void reindeer(int rID, int NR, int TR);
void elf(int eID, int TE);
void santa(int NE, int NR);

