#include "pe_trader.h"

static volatile sig_atomic_t globalFlag = 0;

void sig_handler(int signum);

int main(int argc, char ** argv) {
    if (argc < 2) {
        printf("Not enough arguments\n");
        return 1;
    }

    char exchange[30];
    sprintf(exchange, FIFO_EXCHANGE, atoi(argv[1]));
    char trader[30];
    sprintf(trader, FIFO_TRADER, atoi(argv[1]));

    // connect to named pipes
    // printf("Trader connecting to pipes.\n");
    char temp[2];
    int fd = open(exchange, O_RDONLY, O_NONBLOCK);
    int wd = open(trader, O_WRONLY, O_NONBLOCK);
    // printf("Trader opened FIFOS\n");

    // check for market open

    // event loop:
    int counter = 0;
    while (1) {
        // register signal handler
        signal(SIGUSR1, sig_handler);

        // wait for exchange update (MARKET message)
        if (globalFlag == 1){
            char buffer[200] = {0};
            globalFlag = 0;
            for (int i = 0; i < 200; i++) {
                read(fd, temp, 1);
                buffer[i] = temp[0];
                if (temp[0] == ';') {
                    break;
                }
            }
            // printf("Read String: %s\n", buffer);
            
            if (strncmp(buffer, "MARKET SELL", 11) == 0) {
                char bufferCopy[200];
                strcpy(bufferCopy, buffer);

                // printf("Found market sell\n");
                char *market = strtok(buffer, " ");
                char *orderType = strtok(NULL, " ");
                char *product = strtok(NULL, " ");
                char *quantity = strtok(NULL, " ");
                char *price = strtok(NULL, " ");
                market = orderType;
                orderType = market;
                product = price;
                price = product;

                if (atoi(quantity) > 999) {
                    // printf("[PEX-Milestone] Trader disconnected\n");
                    // close(fd);
                    // close(wd);
                    // return 0;
                    break;
                }
                // printf("Trader sending order\n");
    
                // printf("Read String: %s\n", bufferCopy);

                // send order
                char string[200] = {0};
                strcpy(string, "BUY ");
                char str[6];
                sprintf(str, "%d", counter);
                strcat(string, str);
                for (int i = 0; i < strlen(bufferCopy); i++) if (i > 10) string[strlen(string)] = bufferCopy[i];

                counter += 1;

                // printf("TRADER ORDER: %s\n", string);

                // printf("TRADER SENT ORDER\n");
                write(wd, string, strlen(string));
                kill(getppid(), SIGUSR1);

                // If ordered: keep sending kills until order accepted
                while (1) {
                    signal(SIGUSR1, sig_handler);
                    if (globalFlag == 1) {
                        char buffer[200] = {0};
                        globalFlag = 0;
                        for (int i = 0; i < 200; i++) {
                            read(fd, temp, 1);
                            buffer[i] = temp[0];
                            if (temp[0] == ';') {
                                break;
                            }
                        }
                        
                        char s[50] = {0};
                        sprintf(s, "ACCEPTED %d;", counter - 1);

                        if (strcmp(buffer, s) == 0) break;
                        kill(getppid(), SIGUSR1);
                    }
                }
            }
        }
    }
    close(fd);
    close(wd);
}

void sig_handler(int signum) {
    globalFlag = 1;
}