/* proj2.c
 * autor: Matúš Remeň, FIT
 * prelozene: gcc version 9.3.0 (Ubuntu 9.3.0-17ubuntu1~20.04)
 * popis: Riesenie synchronizacie pre priklad Santa Claus Problem
 * zdroj: Little Book of Semaphores
 */

#include "proj2.h"

args_t read_arguments(int argc, char **argv){
	if (argc != 5){
		fprintf(stderr, "Error: read_arguments: Missing arguments\n");
		exit(1);
	}

	args_t args;

	for (int i = 1; i < argc; ++i){
		char *end;
		int num = (int)strtol(argv[i], &end, 10);
		switch (i){
			case 1:
				if (*end || num < 1 || num >= ELF_LIMIT){
					fprintf(stderr, "Error: read_arguments: Invalid number of elves\n");
					exit(1);
				}
				args.NE = num;
				break;
			case 2:
				if (*end || num < 1 || num >= REINDEER_LIMIT){
					fprintf(stderr, "Error: read_arguments: Invalid number of reindeer\n");
					exit(1);
				}
				args.NR = num;
				break;
			case 3:
				if (*end || num < 0 || num > TIME_LIMIT){
					fprintf(stderr, "Error: read_arguments: Invalid time limit\n");
					exit(1);
				}
				args.TE = num;
				break;
			case 4:
				if (*end || num < 0 || num > TIME_LIMIT){
					fprintf(stderr, "Error: read_arguments: Invalid time limit\n");
					exit(1);
				}
				args.TR = num;
				break;
		}
	}

	return args;
}

// shared memory
int *sh_msg_c;	// message counter
int *sh_elf_c;	// elf counter
int *sh_rdr_c;	// reindeer counter
bool *closed;	// state of santa's workshop

// declaration of semaphores
sem_t *santa_sem;		// manages santa's process
sem_t *elf_sem;			// protects santa's workshop from entering more than 3 elves
sem_t *reindeer_sem;	// manages hitching
sem_t *print_sem;		// manages msg counter and printing to file
sem_t *mutex;			// manages elf counter, reindeer counter and workshop state
sem_t *sigelf_sem;		// wakes elves waiting for santa
sem_t *sigsleep_sem;	// prevents santa from going to sleep before all waiting elves got help
sem_t *sighitch_sem;	// prevents santa from starting christmas before all reindeers got hitched

// semaphore names
const char *santa_sname = "/xremen01_santa";
const char *elf_sname = "/xremen01_elf";
const char *reindeer_sname = "/xremen01_reindeer";
const char *print_sname = "/xremen01_print";
const char *mutex_sname = "/xremen01_mutex";
const char *sigelf_sname = "/xremen01_sigelf";
const char *sigsleep_sname = "/xremen01_sigsleep";
const char *sighitch_sname = "/xremen01_sighitch";

FILE *file;

int init_resources(){
	unsigned int SEM_MODE = 0666;	// rwx for all
	bool return_code = 0;

	// initialization of semaphores	
	santa_sem = sem_open(santa_sname, O_CREAT | O_EXCL, SEM_MODE, LOCKED);
	elf_sem = sem_open(elf_sname, O_CREAT | O_EXCL, SEM_MODE, UNLOCKED);
	reindeer_sem = sem_open(reindeer_sname, O_CREAT | O_EXCL, SEM_MODE, LOCKED);
	print_sem = sem_open(print_sname, O_CREAT | O_EXCL, SEM_MODE, UNLOCKED);
	mutex = sem_open(mutex_sname, O_CREAT | O_EXCL, SEM_MODE, UNLOCKED);
	sigelf_sem = sem_open(sigelf_sname, O_CREAT | O_EXCL, SEM_MODE, LOCKED);
	sigsleep_sem = sem_open(sigsleep_sname, O_CREAT | O_EXCL, SEM_MODE, LOCKED);
	sighitch_sem = sem_open(sighitch_sname, O_CREAT | O_EXCL, SEM_MODE, LOCKED);

	// shared memory
	sh_msg_c = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	sh_elf_c = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	sh_rdr_c = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	closed = mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	file = fopen("proj2.out", "w+");
	
	if (santa_sem == SEM_FAILED || elf_sem == SEM_FAILED ||
			reindeer_sem == SEM_FAILED || print_sem == SEM_FAILED ||
			mutex == SEM_FAILED || sigelf_sem == SEM_FAILED ||
			sigsleep_sem == SEM_FAILED || sighitch_sem == SEM_FAILED){
		fprintf(stderr, "Error: init_resources: Initialization of semaphores failed\n");
		return_code = 1;
	}
	if (sh_msg_c == MAP_FAILED || sh_elf_c == MAP_FAILED || 
			sh_rdr_c == MAP_FAILED || closed == MAP_FAILED){
		fprintf(stderr, "Error: init_resources: Initialization of shared memory failed\n");
		return_code = 1;
	}
	if (file == NULL){
		fprintf(stderr, "Error: main: Could not open file\n");
		return_code = 1;
	}

	*sh_rdr_c = *sh_elf_c = 0;
	*sh_msg_c = 1;
	*closed = false;

	return return_code;
}

void free_resources(){
	sem_close(santa_sem);
	sem_close(elf_sem);
	sem_close(reindeer_sem);
	sem_close(print_sem);
	sem_close(mutex);
	sem_close(sigelf_sem);
	sem_close(sigsleep_sem);
	sem_close(sighitch_sem);

	sem_unlink(santa_sname);
	sem_unlink(elf_sname);
	sem_unlink(reindeer_sname);
	sem_unlink(print_sname);
	sem_unlink(mutex_sname);
	sem_unlink(sigelf_sname);
	sem_unlink(sigsleep_sname);
	sem_unlink(sighitch_sname);

	munmap(sh_msg_c, sizeof(int));
	munmap(sh_elf_c, sizeof(int));
	munmap(sh_rdr_c, sizeof(int));
	munmap(closed, sizeof(bool));

	fclose(file);
}

void generate_elves(int NE, int TE){
	for (int i = 1; i <= NE; ++i){
		pid_t elf_p = fork();
		if (elf_p < 0){
			fprintf(stderr, "Error: generate_elves: Elf fork failed\n");
			free_resources();
			exit(1);
		}
		else if (elf_p == 0){		// elf child
			elf(i, TE);
			exit(0);
		}
	}

	// waiting for end of all children processes
	for (int i = 0; i < NE; ++i)
		wait(NULL);
}

void generate_reindeer(int NR, int TR){
	for (int i = 1; i <= NR; ++i){
		pid_t reindeer_p = fork();
		if (reindeer_p < 0){
			fprintf(stderr, "Error: generate_reindeer: Reindeer fork failed\n");
			exit(1);
		}
		else if (reindeer_p == 0){	// reindeer child
			reindeer(i, NR, TR);
			exit(0);
		}
	}

	// waiting for end of all children processes
	for (int i = 0; i < NR; ++i)
		wait(NULL);
}

unsigned rand_time_elf(unsigned max_time){
	if (max_time == 0)
		return 0;
	srand((unsigned) time(NULL));
	
	return rand() % (max_time+1);	// <0, max_time>
}

unsigned rand_time_reindeer(unsigned max_time){
	if (max_time == 0)
		return 0;
	unsigned half_time = max_time / 2;
	srand((unsigned) time(NULL));
	
	return half_time + rand() % (half_time+1);	// <max_time/2, max_time>
}

void reindeer(int rID, int NR, int TR){
	sem_wait(print_sem);
	fprintf(file, "%d: RD %d: rstarted\n", (*sh_msg_c)++, rID);
	sem_post(print_sem);

	usleep(rand_time_reindeer(TR)*1000);	// time before returning from holiday

	sem_wait(mutex);
	sem_wait(print_sem);
	fprintf(file, "%d: RD %d: return home\n", (*sh_msg_c)++, rID);
	sem_post(print_sem);
	(*sh_rdr_c)++;
	if ((*sh_rdr_c) == NR)
		sem_post(santa_sem);
	sem_post(mutex);

	sem_wait(reindeer_sem);		// waiting for hitch
	
	sem_wait(mutex);
	sem_wait(print_sem);
	fprintf(file, "%d: RD %d: get hitched\n", (*sh_msg_c)++, rID);
	sem_post(print_sem);
	
	(*sh_rdr_c)--;
	
	if (*sh_rdr_c == 0)			// last reindeer lets santa start Christmas
		sem_post(sighitch_sem);
	sem_post(mutex);
}

void elf(int eID, int TE){
	sem_wait(print_sem);
	fprintf(file, "%d: Elf %d: started\n", (*sh_msg_c)++, eID);
	sem_post(print_sem);
	bool santa_available = true;
	
	while (santa_available){
		usleep(rand_time_elf(TE)*1000);		// solo work

		sem_wait(elf_sem);
		sem_wait(mutex);
		sem_wait(print_sem);
		fprintf(file, "%d: Elf %d: need help\n", (*sh_msg_c)++, eID);
		sem_post(print_sem);
		(*sh_elf_c)++;
		if ((*sh_elf_c) == 3)
			sem_post(santa_sem);		// third elf wakes up santa
		else
			sem_post(elf_sem);
		sem_post(mutex);

		sem_wait(sigelf_sem);			// getting help or taking holidays

		sem_wait(mutex);
		if (*closed){					// taking holidays
			santa_available = false;
			(*sh_elf_c)--;
			if ((*sh_elf_c) == 0)
				sem_post(elf_sem);		// last elf leaving lets next elves come to santa's workshop
		}
		else{							// getting help
			sem_wait(print_sem);
			fprintf(file, "%d: Elf %d: get help\n", (*sh_msg_c)++, eID);
			sem_post(print_sem);
			(*sh_elf_c)--;
			if ((*sh_elf_c) == 0){
				sem_post(sigsleep_sem);	// last elf lets santa return to sleep
				sem_post(elf_sem);		// last elf leaving lets next elves come to santa's workshop
			}
		}
		sem_post(mutex);
	}
	
	sem_wait(print_sem);
	fprintf(file, "%d: Elf %d: taking holidays\n", (*sh_msg_c)++, eID);
	sem_post(print_sem);
}

void santa(int NE, int NR){
	bool available = true;
	
	while (available){
		sem_wait(print_sem);
		fprintf(file, "%d: Santa: going to sleep\n", (*sh_msg_c)++);
		sem_post(print_sem);
		
		sem_wait(santa_sem);		// waiting for wake up

		sem_wait(mutex);
		// checking who woke up santa
		if (*sh_rdr_c >= NR){		// reindeer
			sem_wait(print_sem);
			fprintf(file, "%d: Santa: closing workshop\n", (*sh_msg_c)++);
			sem_post(print_sem);
		
			*closed = true;
			available = false;
			for (int i = 0; i < NR; ++i){
				sem_post(reindeer_sem);
			}
			for (int i = 0; i < NE; ++i){
				sem_post(sigelf_sem);
			}
			sem_post(mutex);
		}
		else if (*sh_elf_c == 3){	// elves
			sem_wait(print_sem);
			fprintf(file, "%d: Santa: helping elves\n", (*sh_msg_c)++);
			sem_post(print_sem);
			for (int i = 0; i < 3; ++i){	// helping elves
				sem_post(sigelf_sem);
			}
			sem_post(mutex);
			sem_wait(sigsleep_sem);	// waiting for last elf to leave
		}
	}

	sem_wait(sighitch_sem);			// waiting for last reindeer to get hitched
	sem_wait(print_sem);
	fprintf(file, "%d: Santa: Christmas started\n", (*sh_msg_c)++);
	sem_post(print_sem);
}

/* ====================================================================== */

int main(int argc, char **argv){
	setbuf(stderr, NULL);			// turns off message buffering

	args_t args = read_arguments(argc, argv);

	if (init_resources()){
		free_resources();
		return 1;
	}
	setbuf(file, NULL);

	pid_t santa_p = fork();
	
	if (santa_p < 0){
		fprintf(stderr, "Error: main: Santa fork failed\n");
		free_resources();	
		return 1;
	}
	else if (santa_p == 0){			// child process of santa
		santa(args.NE, args.NR);
		exit(0);
	}
	else{							// parent process, create elves
		pid_t elf_gen = fork();
		if (elf_gen < 0){
			fprintf(stderr, "Error: main: Elf generator fork failed\n");
			free_resources();
			return 1;
		}
		else if (elf_gen == 0){		// elf generator child
			generate_elves(args.NE, args.TE);
			exit(0);
		}
		else{
			pid_t reindeer_gen = fork();
			if (reindeer_gen < 0){
				fprintf(stderr, "Error: main: Reindeer generator fork failed\n");
				free_resources();
				return 1;
			}
			else if (reindeer_gen == 0){	// reindeer generator child
				generate_reindeer(args.NR, args.TR);
				exit(0);
			}
		}
	}

	wait(NULL);
	wait(NULL);
	wait(NULL);
	free_resources();
	
	return 0;
}

