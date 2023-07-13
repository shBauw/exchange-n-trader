#ifndef PE_COMMON_H
#define PE_COMMON_H

#define _POSIX_C_SOURCE 199309L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/select.h>
// #include "functions.h"



#define FIFO_EXCHANGE "/tmp/pe_exchange_%d"
#define FIFO_TRADER "/tmp/pe_trader_%d"
#define FEE_PERCENTAGE 1


struct trader {
	pid_t pid;
	int num;
	int exchange;
	int trader;
	int exited;
	int counter;
	long *positions;
	long *balance;
	struct pidAmt *traderOrder;
};

struct pidAmt {
	int counter;
	int amount;
	int filled;
	int total;
	struct trader *trader;
	struct orderLevel *product;
	struct pidAmt *traderNext;
	struct pidAmt *traderPrev;	
	struct pidAmt *productNext;
	struct pidAmt *productPrev;
};

struct orderLevel {
	int price;
	int buySell;
	int name;
	struct pidAmt *pidAmt;
	struct orderLevel *next;
	struct orderLevel *prev;
};

struct counters {
	long fees;
	int total;
	int exitCounter;
};


#endif
