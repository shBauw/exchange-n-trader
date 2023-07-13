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

    signal(SIGUSR1, sig_handler);

    while (globalFlag == 0);
    signal(SIGUSR1, sig_handler);
    if (globalFlag != 0) {
        globalFlag = 0;
        char buffer[200] = {0};
        for (int i = 0; i < 200; i++) {
            read(fd, temp, 1);
            buffer[i] = temp[0];
            if (temp[0] == ';') {
                break;
            }
        }
        if (strcmp(buffer, "MARKET OPEN;") != 0) {
            printf("TRADER 1 ERROR\n");
            return 1;
        }
        char string[200] = "BUY;";
        write(wd, string, strlen(string));
        kill(getppid(), SIGUSR1);
        printf("[T1]\t%s\n", buffer);
    }

    while (globalFlag == 0);
    signal(SIGUSR1, sig_handler);
    if (globalFlag != 0) {
        globalFlag = 0;
        char buffer[200] = {0};
        for (int i = 0; i < 200; i++) {
            read(fd, temp, 1);
            buffer[i] = temp[0];
            if (temp[0] == ';') {
                break;
            }
        }
        if (strcmp(buffer, "INVALID;") != 0) {
            printf("TRADER 1 ERROR\n");
            return 1;
        }
        char string[200] = "BUY 0 GPU 100 100;";
        write(wd, string, strlen(string));
        kill(getppid(), SIGUSR1);
        printf("[T1]\t%s\n", buffer);
    }

    while (globalFlag == 0);
    signal(SIGUSR1, sig_handler);
    if (globalFlag != 0) {
        globalFlag = 0;
        char buffer[200] = {0};
        for (int i = 0; i < 200; i++) {
            read(fd, temp, 1);
            buffer[i] = temp[0];
            if (temp[0] == ';') {
                break;
            }
        }
        if (strcmp(buffer, "ACCEPTED 0;") != 0) {
            printf("TRADER 1 ERROR\n");
            return 1;
        }
        char string[200] = "SELL 1 Router 100 100;";
        write(wd, string, strlen(string));
        kill(getppid(), SIGUSR1);
        printf("[T1]\t%s\n", buffer);
    }

    while (globalFlag == 0);
    signal(SIGUSR1, sig_handler);
    if (globalFlag != 0) {
        globalFlag = 0;
        char buffer[200] = {0};
        for (int i = 0; i < 200; i++) {
            read(fd, temp, 1);
            buffer[i] = temp[0];
            if (temp[0] == ';') {
                break;
            }
        }
        if (strcmp(buffer, "ACCEPTED 1;") != 0) {
            printf("TRADER 1 ERROR\n");
            return 1;
        }
        char string[200] = "AMEND 1 1 1;";
        write(wd, string, strlen(string));
        kill(getppid(), SIGUSR1);
        printf("[T1]\t%s\n", buffer);
    }
    

    while (globalFlag == 0);
    signal(SIGUSR1, sig_handler);
    if (globalFlag != 0) {
        globalFlag = 0;
        char buffer[200] = {0};
        for (int i = 0; i < 200; i++) {
            read(fd, temp, 1);
            buffer[i] = temp[0];
            if (temp[0] == ';') {
                break;
            }
        }
        if (strcmp(buffer, "AMENDED 1;") != 0) {
            printf("TRADER 1 ERROR\n");
            return 1;
        }
        char string[200] = "CANCEL 1;";
        write(wd, string, strlen(string));
        kill(getppid(), SIGUSR1);
        printf("[T1]\t%s\n", buffer);
    }

    while (globalFlag == 0);
    signal(SIGUSR1, sig_handler);
    if (globalFlag != 0) {
        globalFlag = 0;
        char buffer[200] = {0};
        for (int i = 0; i < 200; i++) {
            read(fd, temp, 1);
            buffer[i] = temp[0];
            if (temp[0] == ';') {
                break;
            }
        }
        if (strcmp(buffer, "CANCELLED 1;") != 0) {
            printf("TRADER 1 ERROR\n");
            return 1;
        }
        printf("[T1]\t%s\n", buffer);
    }

    while (globalFlag == 0);
    signal(SIGUSR1, sig_handler);
    if (globalFlag != 0) {
        globalFlag = 0;
        char buffer[200] = {0};
        for (int i = 0; i < 200; i++) {
            read(fd, temp, 1);
            buffer[i] = temp[0];
            if (temp[0] == ';') {
                break;
            }
        }
        if (strcmp(buffer, "MARKET SELL GPU 100 100;") != 0) {
            printf("TRADER 1 ERROR\n");
            return 1;
        }
        printf("[T1]\t%s\n", buffer);
    }

    while (globalFlag == 0);
    if (globalFlag != 0) {
        globalFlag = 0;
        char buffer[200] = {0};
        for (int i = 0; i < 200; i++) {
            read(fd, temp, 1);
            buffer[i] = temp[0];
            if (temp[0] == ';') {
                break;
            }
        }
        if (strcmp(buffer, "FILL 0 100;") != 0) {
            printf("TRADER 1 ERROR\n");
            return 1;
        }
        printf("[T1]\t%s\n", buffer);
    }
}

void sig_handler(int signum) {
    globalFlag = 1;
}