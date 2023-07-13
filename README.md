1. Describe how your exchange works.
My exchange forks, using execv to create child processes, communicating with them through named pipes and signals.
The exchange itself uses linked lists to use the same nodes in product lists (categorised by price and buy/sell) and trader lists (categorised by the trader order). This creates an effective interweaved web which allows for effective processing or orders and balancing + filling them.

2. Describe your design decisions for the trader and how it's fault-tolerant.
The trader is fault tolerant as it accounts for various errors. These include NULL pointers, incorrent/malformed commands, signals from wrong sources and buffer overflows and underflows. It also checks the returns of exec, fork and waitpid to ensure there are no errors.
It is efficient due to the linked lists. While this may increase the space complexity mildly due to the various categorisations, this is in fact very effective for the time complexity, with the system being designed so that orders can be stored and only running balance once will effeciently fill as many as possible.

3. Describe your tests and how to run them.
I created a trader that I would manually input commands into to test for edge cases.
Additionally, I created test traders (trader_test1, trader_test2), launched using the regular command (./pe_exchange products.txt tests/trader_test1 tests/trader_test2). While there is no distinct unit test comparison, this checks that all functions work as expected, with the expected final output written below. test1 runs all basic functions and after a final cancel order, test 2 runs to fill the orders, then both disconnect.

This produces the final profit and orderbook/positions as shown below.

[PEX]   --ORDERBOOK--
[PEX]   Product: GPU; Buy levels: 0; Sell levels: 0
[PEX]   Product: Router; Buy levels: 0; Sell levels: 0
[PEX]   --POSITIONS--
[PEX]   Trader 0: GPU 100 ($-10000), Router 0 ($0)
[PEX]   Trader 1: GPU -100 ($9900), Router 0 ($0)
[PEX] Trader 0 disconnected
[PEX] Trader 1 disconnected
[PEX] Trading completed
[PEX] Exchange fees collected: $100

It becomes difficult to measure specific output so I found testing all functionalities in this way to be more efficient.