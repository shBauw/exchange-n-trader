#include "functions.h"

void invalid(struct trader trader) {
	char inv[20] = "INVALID;";
	write(trader.exchange, inv, strlen(inv));
	kill(trader.pid, SIGUSR1);
}

void clean(struct pidAmt *pid_amt) {
	if (pid_amt->trader->traderOrder == pid_amt) pid_amt->trader->traderOrder = pid_amt->traderNext;
	if (pid_amt->traderNext != NULL) pid_amt->traderNext->traderPrev = pid_amt->traderPrev;
	if (pid_amt->traderPrev != NULL) pid_amt->traderPrev->traderNext = pid_amt->traderNext;
	if (pid_amt->productNext != NULL) pid_amt->productNext->productPrev = pid_amt->productPrev;
	if (pid_amt->productPrev != NULL) pid_amt->productPrev->productNext = pid_amt->productNext;
	else pid_amt->product->pidAmt = pid_amt->productNext;
	if (pid_amt->product->pidAmt == NULL) {
		if (pid_amt->product->prev != NULL) pid_amt->product->prev->next = pid_amt->product->next;
		if (pid_amt->product->next != NULL) pid_amt->product->next->prev = pid_amt->product->prev;
		free(pid_amt->product);
	}
	free(pid_amt);
}

void send(struct pidAmt *pid_amt, int amount) {
	if (pid_amt->trader->exited == 0) {
		char s[20];
		sprintf(s, "FILL %d %d;", pid_amt->counter, amount);
		write(pid_amt->trader->exchange, s, strlen(s));
		kill(pid_amt->trader->pid, SIGUSR1);
	}
}

long match(struct pidAmt *buy, struct pidAmt *sell, int amount, int item) {
	int price;
	int counter1;
	int counter2;
	int trader1;
	int trader2;

	if (buy->total > sell->total) {
		price = sell->product->price;
		counter1 = sell->counter;
		trader1 = sell->trader->num;
		counter2 = buy->counter;
		trader2 = buy->trader->num;
	} else {
		price = buy->product->price;
		counter2 = sell->counter;
		trader2 = sell->trader->num;
		counter1 = buy->counter;
		trader1 = buy->trader->num;
	}

	long value = (long) price * (long) amount;
	long fee = 0.01 * value;
	int rnd = value % 100;
	if (rnd > 49) fee += 1;

	printf("[PEX] Match: Order %d [T%d], New Order %d [T%d], value: $%ld, fee: $%ld.\n", counter1, trader1, counter2, trader2, value, fee);

	buy->trader->positions[item] += amount;
	sell->trader->positions[item] -= amount;

	if (buy->total > sell->total) {
		buy->trader->balance[item] -= value + fee;
		sell->trader->balance[item] += value;
	} else {
		buy->trader->balance[item] -= value;
		sell->trader->balance[item] += value - fee;
	}

	return fee;
}

long balance(struct orderLevel *buy, struct orderLevel *sell, int item) {
	long fees = 0;
	buy = buy->next;
	sell = sell->next;
	struct orderLevel *buy2 = buy;
	struct orderLevel *sell2 = sell;
	if (buy2 == NULL || sell2 == NULL) return fees;
	while (sell->next != NULL) sell = sell->next;

	struct pidAmt *bPA = buy->pidAmt;
	struct pidAmt *sPA = sell->pidAmt;

	while (buy->price > sell->price - 1) {
		while (bPA != NULL && sPA != NULL) {
			bPA->filled = -1;
			sPA->filled = -1;
			int bAm = bPA->amount;
			int sAm = sPA->amount;

			if (bAm > sAm) {
				fees += match(bPA, sPA, sAm, item);

				bPA->amount -= sAm;
				sPA->amount -= sAm;
				
				send(bPA, sAm);
				send(sPA, sAm);

				sPA = sPA->productNext;
			} else if (bAm < sAm) {
				fees += match(bPA, sPA, bAm, item);

				bPA->amount -= bAm;
				sPA->amount -= bAm;

				send(bPA, bAm);
				send(sPA, bAm);

				bPA = bPA->productNext;
			} else {
				fees += match(bPA, sPA, bAm, item);

				bPA->amount -= bAm;
				sPA->amount -= bAm;

				send(bPA, bAm);
				send(sPA, bAm);
				
				sPA = sPA->productNext;
				bPA = bPA->productNext;
			}
		}
		if (bPA == NULL) buy = buy->next;
		if (sPA == NULL) sell = sell->prev;
		if (buy == NULL || sell->prev == NULL) break;
		bPA = buy->pidAmt;
		while (bPA->amount == 0) {
			bPA = bPA->productNext;
			if (bPA == NULL) break;
		}
		sPA = sell->pidAmt;
		while (sPA->amount == 0) {
			sPA = sPA->productNext;
			if (sPA == NULL) break;
		}
	}

	buy = buy2;
	while (buy != NULL) {
		struct pidAmt *bPA = buy->pidAmt;
		struct orderLevel *bNext = buy->next;
		while (bPA != NULL) {
			struct pidAmt *next = bPA->productNext;
			if (0 == bPA->amount) clean(bPA);
			bPA = next;
		}
		buy = bNext;
	}
	sell = sell2;
	while (sell != NULL) {
		struct pidAmt *sPA = sell->pidAmt;
		struct orderLevel *sNext = sell->next;
		while (sPA != NULL) {
			struct pidAmt *next = sPA->productNext;
			if (0 == sPA->amount) clean(sPA);
			sPA = next;
		}
		sell = sNext;
	}

	return fees;
}

void add(struct pidAmt *pid_amt, struct orderLevel *start, char *price, int buySell, int name) {
	int made = 0;
	while (start->next != NULL) {
		if (start->next->price < atoi(price)) {
			made = 1;
			struct orderLevel *newLevel = (struct orderLevel *) malloc(sizeof(struct orderLevel));
			newLevel->name = name;
			newLevel->buySell = buySell;
			newLevel->next = start->next;
			start->next->prev = newLevel;
			newLevel->prev = start;
			start->next = newLevel;
			newLevel->price = atoi(price);
			newLevel->pidAmt = pid_amt;
			pid_amt->product = newLevel;
			break;
		} else if (start->next->price == atoi(price)) {
			made = 1;
			start = start->next;
			struct pidAmt *prev = start->pidAmt;
			while (prev->productNext != NULL) {
				prev = prev->productNext;
			}
			prev->productNext = pid_amt;
			pid_amt->productPrev = prev;
			pid_amt->product = start;
			break;
		}
		start = start->next;
	}
	if (made == 0) {
		struct orderLevel *newLevel = (struct orderLevel *) malloc(sizeof(struct orderLevel));
		newLevel->name = name;
		newLevel->buySell = buySell;
		newLevel->next = NULL;
		newLevel->prev = start;
		start->next = newLevel;
		newLevel->price = atoi(price);
		newLevel->pidAmt = pid_amt;
		pid_amt->product = newLevel;
	}
}

void count(struct orderLevel *order, int sb) {
	while(order != NULL) {
		int amount = 0;
		int orderCount = 0;

		struct pidAmt *pid_amt = order->pidAmt;
		while (pid_amt != NULL) {
			amount += pid_amt->amount;
			orderCount += 1;
			pid_amt = pid_amt->productNext;
		}
		
		char str[5] = {0};

		if (sb == 0) strcpy(str, "SELL");
		else strcpy(str, "BUY");
		if (orderCount == 1) printf("[PEX]\t\t%s %d @ $%d (1 order)\n", str, amount, order->price);
		else printf("[PEX]\t\t%s %d @ $%d (%d orders)\n", str, amount, order->price, orderCount);

		order = order->next;
	}
}

void orderbook(struct orderLevel **orders, char **items, int length) {
	printf("[PEX]\t--ORDERBOOK--\n");
	for (int i = 0; i < length; i++) {
		int buyCount = 0;
		struct orderLevel *levels = orders[i*2];
		while (levels->next != NULL) {
			buyCount += 1;
			levels = levels->next;
		}
		int sellCount = 0;
		levels = orders[i*2 + 1];
		while (levels->next != NULL) {
			sellCount += 1;
			levels = levels->next;
		}
		printf("[PEX]\tProduct: %s; Buy levels: %d; Sell levels: %d\n", items[i], buyCount, sellCount);

		struct orderLevel *buyLevels = orders[i*2]->next;
		struct orderLevel *sellLevels = orders[i*2+1]->next;

		count(sellLevels, 0);
		count(buyLevels, 1);
	}
}

void positions(struct trader *traders, int length, int len, char **items) {
	printf("[PEX]\t--POSITIONS--\n");
	for (int i = 0; i < length; i++) {
		printf("[PEX]\tTrader %d:", i);
		for (int j = 0; j < len; j++) {
			printf(" %s %ld ($%ld)", items[j], traders[i].positions[j], traders[i].balance[j]);
			if (j != len - 1) printf(",");
		}
		printf("\n");
	}
}

void freeAll(struct trader *traders, char **items, struct orderLevel **orders, int argc, int length) {
	// Close and unlink all FIFOS
	for (int i = 0; i < argc - 2; i++) {
		close(traders[i].exchange);
		close(traders[i].trader);

		char exchange[30];
		sprintf(exchange, FIFO_EXCHANGE, i);
		char trader[30];
		sprintf(trader, FIFO_TRADER, i);
		
		unlink(exchange);
		unlink(trader);
	}

	// Free memory
	for (int i = 0; i < argc - 2; i++) {
		free(traders[i].positions);
		free(traders[i].balance);
	}
	free(traders);
	for (int i = 0; i < length; i++) {
		free(items[i]);
	}
	free(items);
	for (int i = 0; i < length * 2; i++) {
		struct orderLevel *orderStart = orders[i];
		while (1) {
			if (orderStart->pidAmt != NULL) {
				struct pidAmt *paStart = orderStart->pidAmt;
				while (paStart->productNext != NULL) {
					paStart = paStart->productNext;
					free(paStart->productPrev);
				}
				free(paStart);
			}
			
			if (orderStart->next != NULL) {
				orderStart = orderStart->next;
				free(orderStart->prev);
			} else break;
		}
		free(orderStart);
	}
	free(orders);
}

int exited(struct trader *traders, int exitCounter, int argc, long fees) {
	for (int i = 0; i < argc - 2; i++) {
		if (traders[i].exited == 1) continue;

		int status;
		pid_t exited = waitpid(traders[i].pid, &status, WNOHANG);
		if (exited == traders[i].pid) {
			traders[i].exited = 1;
			printf("[PEX] Trader %d disconnected\n", i);
			exitCounter += 1;
		}
	}
	// If all traders exited, finish trading
	if (exitCounter == argc - 2) {
		printf("[PEX] Trading completed\n");
		printf("[PEX] Exchange fees collected: $%ld\n", fees);
		return 0;
	}
	return 1;
}

int initt(struct trader *traders, int argc, char **argv, int length) {
	for (int i = 2; i < argc; i++) {
		// Create filenames
		char exchange[30];
		sprintf(exchange, FIFO_EXCHANGE, i-2);
		char trader[30];
		sprintf(trader, FIFO_TRADER, i-2);
		
		// Unlink before making for cleanup (don't care about returns)
		unlink(exchange);
		unlink(trader);

		// Make FIFOs, checking for errors
		if (mkfifo(exchange, 0666) == -1) {
			if (errno != EEXIST) {
				printf("FIFO error\n");
				return 1;
			}
		}
		printf("[PEX] Created FIFO %s\n", exchange);
		if (mkfifo(trader, 0666) == -1) {
			if (errno != EEXIST) {
				printf("FIFO error\n");
				return 1;
			}
		}
		printf("[PEX] Created FIFO %s\n", trader);

		// Execute traders
		char str[12];
  		sprintf(str, "%d", i - 2);
		printf("[PEX] Starting trader %d (%s)\n", i - 2, argv[i]);
		pid_t p = fork();
		char *args[]={argv[i], str ,NULL};
		if (p == 0) {
			execv(args[0], args);
			// This is the error check as execv would replace the running program
			printf("[PEX] Error with starting trader %d (%s)\n", i - 2, argv[i]);
			return 1;
		} else {
			// Set trader object
			traders[i-2].balance = (long *) malloc((sizeof(long)) * length);
			traders[i-2].positions = (long *) malloc(sizeof(long) * length);
			for (int k = 0; k < length; k++) {
				traders[i-2].balance[k] = 0;
				traders[i-2].positions[k] = 0;
			}
			traders[i-2].num = i - 2;
			traders[i-2].exited = 0;
			traders[i-2].pid = p;
			traders[i-2].exchange = open(exchange, O_WRONLY);
			printf("[PEX] Connected to %s\n", exchange);
			traders[i-2].trader = open(trader, O_RDONLY);
			printf("[PEX] Connected to %s\n", trader);
			traders[i-2].counter = 0;
			traders[i-2].traderOrder = NULL;
		}
		fflush(stdout);
	}
	return 0;
}

void buy(struct trader *traders, struct orderLevel **orders, char **items, long fees, int total, int z, int length, char *quantity, char *product, char *price, char *order_id, int argc) {
	if (product == NULL || quantity == NULL || price == NULL) {
		invalid(traders[z]);
		return;
	}
	if (atoi(quantity) < 1 || atoi(price) < 1 || atoi(quantity) > 999999 || atoi(price) > 999999) invalid(traders[z]);
	else {
		int found = 0;
		for (int i = 0; i < length; i++) {
			if (strcmp(product, items[i]) == 0) {
				found = 1;
				char s[20] = {0};
				sprintf(s, "ACCEPTED %d;", atoi(order_id));
				write(traders[z].exchange, s, strlen(s));
				kill(traders[z].pid, SIGUSR1);
				
				// Base
				struct pidAmt *pid_amt = (struct pidAmt *) malloc(sizeof(struct pidAmt));
				pid_amt->trader = &traders[z];
				pid_amt->amount = atoi(quantity);
				pid_amt->filled = 0;
				pid_amt->counter = traders[z].counter;
				pid_amt->total = total;

				// Trader
				pid_amt->traderNext = NULL;
				// Trader order (sorted by when placed)
				if (traders[z].traderOrder == NULL) {
					pid_amt->traderPrev = NULL;
					traders[z].traderOrder = pid_amt;
				} else {
					struct pidAmt *prev = traders[z].traderOrder;
					while (prev->traderNext != NULL) prev = prev->traderNext;
					prev->traderNext = pid_amt;
					pid_amt->traderPrev = prev;
				}

				// Orderlevel order (sorted by highest price)
				pid_amt->productNext = NULL;
				pid_amt->productPrev = NULL;
				struct orderLevel *start = orders[i*2];
				add(pid_amt, start, price, 0, i);
				traders[z].counter += 1;
				total += 1;

				char broadcastOrder[100];
				sprintf(broadcastOrder, "MARKET BUY %s %d %d;", product, atoi(quantity), atoi(price));
				for (int j = 0; j < argc - 2; j++) {
					if (j == z) continue;
					if (traders[j].exited == 1) continue;
					write(traders[j].exchange, broadcastOrder, strlen(broadcastOrder));
					kill(traders[j].pid, SIGUSR1);
				}

				for (int k = 0; k < length; k++) fees += balance(orders[k*2], orders[k*2+1], k);
				orderbook(orders, items, length);
				positions(traders, argc - 2, length, items);

				break;
			}
		}
		if (found == 0) invalid(traders[z]);
	}
}

void sell(struct trader *traders, struct orderLevel **orders, char **items, long fees, int total, int z, int length, char *quantity, char *product, char *price, char *order_id, int argc) {
	if (product == NULL || quantity == NULL || price == NULL) {
		invalid(traders[z]);
		return;
	}
	if (atoi(quantity) < 1 || atoi(price) < 1 || atoi(quantity) > 999999 || atoi(price) > 999999) invalid(traders[z]);
	else {
		int found = 0;
		for (int i = 0; i < length; i++) {
			if (strcmp(product, items[i]) == 0) {
				found = 1;
				char s[20] = {0};
				sprintf(s, "ACCEPTED %d;", atoi(order_id));
				write(traders[z].exchange, s, strlen(s));
				kill(traders[z].pid, SIGUSR1);
				
				// Base
				struct pidAmt *pid_amt = (struct pidAmt *) malloc(sizeof(struct pidAmt));
				pid_amt->trader = &traders[z];
				pid_amt->amount = atoi(quantity);
				pid_amt->filled = 0;
				pid_amt->counter = traders[z].counter;
				pid_amt->total = total;

				// Trader
				pid_amt->traderNext = NULL;
				// Trader order (sorted by when placed)
				if (traders[z].traderOrder == NULL) {
					pid_amt->traderPrev = NULL;
					traders[z].traderOrder = pid_amt;
				} else {
					struct pidAmt *prev = traders[z].traderOrder;
					while (prev->traderNext != NULL) prev = prev->traderNext;
					prev->traderNext = pid_amt;
					pid_amt->traderPrev = prev;
				}

				// Orderlevel order (sorted by highest price)
				pid_amt->productNext = NULL;
				pid_amt->productPrev = NULL;
				struct orderLevel *start = orders[i*2 + 1];
				add(pid_amt, start, price, 1, i);
				traders[z].counter += 1;
				total += 1;

				char broadcastOrder[100];
				sprintf(broadcastOrder, "MARKET SELL %s %d %d;", product, atoi(quantity), atoi(price));
				for (int j = 0; j < argc - 2; j++) {
					if (j == z) continue;
					if (traders[j].exited == 1) continue;
					write(traders[j].exchange, broadcastOrder, strlen(broadcastOrder));
					kill(traders[j].pid, SIGUSR1);
				}

				for (int k = 0; k < length; k++) fees += balance(orders[k*2], orders[k*2+1], k);
				orderbook(orders, items, length);
				positions(traders, argc - 2, length, items);

				break;
			}
		}
		if (found == 0) invalid(traders[z]);
	}
}

void amend(struct trader *traders, struct orderLevel **orders, char **items, long fees, int total, int z, int length, char *quantity, char *price, char *order_id, int argc) {
	if (order_id == NULL || quantity == NULL || price == NULL) {
		invalid(traders[z]);
		return;
	}
	if (atoi(quantity) < 1 || atoi(price) < 1 || atoi(quantity) > 999999 || atoi(price) > 999999) invalid(traders[z]);
	else {
		struct pidAmt *pid_amt = traders[z].traderOrder;
		int amended = 0;
		while(pid_amt != NULL) {
			if (pid_amt->counter == atoi(order_id)) {
				if (pid_amt->filled == -1) break;

				char s2[100];
				if (pid_amt->product->buySell == 0) sprintf(s2, "MARKET BUY %s %d %d;", items[pid_amt->product->name], atoi(quantity), atoi(price));
				else sprintf(s2, "MARKET SELL %s %d %d;", items[pid_amt->product->name], atoi(quantity), atoi(price));

				if (pid_amt->product->price == atoi(price)) {
					pid_amt->amount = atoi(quantity);
					pid_amt->total = total;
				} else {
					struct pidAmt *new = (struct pidAmt *) malloc(sizeof(struct pidAmt));
					new->trader = pid_amt->trader;
					new->amount = atoi(quantity);
					new->counter = pid_amt->counter;
					new->total = total;
					new->filled = pid_amt->filled;
					new->product = NULL;
					new->productNext = NULL;
					new->productPrev = NULL;
					new->traderNext = pid_amt->traderNext;
					new->traderPrev = pid_amt->traderPrev;
					if (pid_amt->traderNext != NULL) pid_amt->traderNext->traderPrev = new;
					if (pid_amt->traderPrev != NULL) pid_amt->traderPrev->traderNext = new;
					pid_amt->traderNext = NULL;
					pid_amt->traderPrev = NULL;
					if (pid_amt == traders[z].traderOrder) traders[z].traderOrder = new;
					struct orderLevel *product = pid_amt->product;
					while (product->prev != NULL) product = product->prev;
					int buySell = pid_amt->product->buySell;
					int i = pid_amt->product->name;
					clean(pid_amt);
					add(new, product, price, buySell, i);
				}

				for (int l = 0; l < argc - 2; l++) {
					if (l == z) continue;
					write(traders[l].exchange, s2, strlen(s2));
					kill(traders[l].pid, SIGUSR1);
				}

				char s[20];
				sprintf(s, "AMENDED %d;", atoi(order_id));
				write(traders[z].exchange, s, strlen(s));
				kill(traders[z].pid, SIGUSR1);
				amended = 1;
				total += 1;
				break;
			}
			pid_amt = pid_amt->traderNext;
		}
		if (amended == 0) invalid(traders[z]);
		else {
			for (int k = 0; k < length; k++) fees += balance(orders[k*2], orders[k*2+1], k);
			orderbook(orders, items, length);
			positions(traders, argc - 2, length, items);
		}
	}
}

void cancel(struct trader *traders, struct orderLevel **orders, char **items, long fees, int total, int z, int length, char *order_id, int argc) {
	if (order_id == NULL) {
		invalid(traders[z]);
		return;
	}
	if (traders[z].traderOrder != NULL) {
		struct pidAmt *pid_amt = traders[z].traderOrder;
		int cancelled = 0;
		while(pid_amt != NULL) {
			if (pid_amt->counter == atoi(order_id)) {
				if (pid_amt->filled == -1) break;
				cancelled = 1;
				
				char s[20];
				sprintf(s, "CANCELLED %d;", atoi(order_id));
				write(traders[z].exchange, s, strlen(s));
				kill(traders[z].pid, SIGUSR1);
				
				char s2[100];
				if (pid_amt->product->buySell == 0) sprintf(s2, "MARKET BUY %s 0 0;", items[pid_amt->product->name]);
				else sprintf(s2, "MARKET SELL %s 0 0;", items[pid_amt->product->name]);

				for (int l = 0; l < argc - 2; l++) {
					if (l == z) continue;
					write(traders[l].exchange, s2, strlen(s2));
					kill(traders[l].pid, SIGUSR1);
				}

				clean(pid_amt);
				break;
			}
			pid_amt = pid_amt->traderNext;
		}
		if (cancelled == 0) invalid(traders[z]);
		else {
			orderbook(orders, items, length);
			positions(traders, argc - 2, length, items);
		}
	} else invalid(traders[z]);
}

void check(struct trader *traders, char **items, struct orderLevel **orders, int argc, int length, pid_t tpid, int total, long fees) {
	int z;
	int parsed = 0;
	char order[200] = {0};
	for (z = 0; z < argc - 2; z++){
		// Figure out which trader placed order
		if (traders[z].pid == tpid) {
			// Parse order
			char temp[2];
			for (int j = 0; j < 200; j++) {
				// If not ending with ;, return invalid
				if (read(traders[z].trader, temp, 1) < 1) {
					invalid(traders[z]);
					break;
				}
				
				// Successfully parsing command
				order[j] = temp[0];
				if (temp[0] == ';') {
					order[j] = '\0';
					printf("[PEX] [T%d] Parsing command: <%s>\n", z, order);
					order[j] = ';';
					order [j+1] = '\0';
					parsed = 1;
					break;
				}
			}
			break;
		}
	}
	if (parsed == 0) return;
	
	// Figure out order type
	char *command = strtok(order, " ");
	if (command == NULL) {
		invalid(traders[z]);
		return;
	}
	// Buy order
	if (strcmp(command, "BUY") == 0) {
		char *order_id = strtok(NULL, " ");
		if (order_id == NULL) {
			invalid(traders[z]);
			return;
		}
		if (atoi(order_id) != traders[z].counter) invalid(traders[z]);
		else{
			char *product = strtok(NULL, " ");
			char *quantity = strtok(NULL, " ");
			char *price = strtok(NULL, " ");

			buy(traders, orders, items, fees, total, z, length, quantity, product, price, order_id, argc);
		}
	// Sell order
	} else if (strcmp(command, "SELL") == 0) {
		char *order_id = strtok(NULL, " ");
		if (order_id == NULL) {
			invalid(traders[z]);
			return;
		}
		if (atoi(order_id) != traders[z].counter) invalid(traders[z]);
		else{
			char *product = strtok(NULL, " ");
			char *quantity = strtok(NULL, " ");
			char *price = strtok(NULL, " ");
			
			sell(traders, orders, items, fees, total, z, length, quantity, product, price, order_id, argc);
		}
	// Amend order
	} else if (strcmp(command, "AMEND") == 0) {
		char *order_id = strtok(NULL, " ");
		char *quantity = strtok(NULL, " ");
		char *price = strtok(NULL, " ");
		
		amend(traders, orders, items, fees, total, z, length, quantity, price, order_id, argc);
	// Cancel order
	} else if (strcmp(command, "CANCEL") == 0) {
		char *order_id = strtok(NULL, " ");
		
		cancel(traders, orders, items, fees, total, z, length, order_id, argc);
	// Invalid if none
	} else {
		invalid(traders[z]);
	}
}
