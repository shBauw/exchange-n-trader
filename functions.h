#ifndef FUNCTIONS_INCLUDED
#define FUNCTIONS_INCLUDED

#include "pe_common.h"

void invalid(struct trader trader);
void clean(struct pidAmt *pid_amt);
void send(struct pidAmt *pid_amt, int amount);
long match(struct pidAmt *buy, struct pidAmt *sell, int amount, int item);
long balance(struct orderLevel *buy, struct orderLevel *sell, int item);
void add(struct pidAmt *pid_amt, struct orderLevel *start, char *price, int buySell, int name);
void count(struct orderLevel *order, int sb);
void orderbook(struct orderLevel **orders, char **items, int length);
void positions(struct trader *traders, int length, int len, char **items);
void freeAll(struct trader *traders, char **items, struct orderLevel **orders, int argc, int length);
int exited(struct trader *traders, int exitCounter, int argc, long fees);
int initt(struct trader *traders, int argc, char **argv, int length);
void buy(struct trader *traders, struct orderLevel **orders, char **items, long fees, int total, int z, int length, char *quantity, char *product, char *price, char *order_id, int argc);
void sell(struct trader *traders, struct orderLevel **orders, char **items, long fees, int total, int z, int length, char *quantity, char *product, char *price, char *order_id, int argc);
void amend(struct trader *traders, struct orderLevel **orders, char **items, long fees, int total, int z, int length, char *quantity, char *price, char *order_id, int argc);
void cancel(struct trader *traders, struct orderLevel **orders, char **items, long fees, int total, int z, int length, char *order_id, int argc);
void check(struct trader *traders, char **items, struct orderLevel **orders, int argc, int length, pid_t tpid, int total, long fees);

#endif