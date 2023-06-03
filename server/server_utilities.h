#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#define SIG_FAIL -6
#define SIG_SUCCESS -5
#define SIG_LOWBID -4 // Bidding money lower than current price + min increment
#define SIG_NEM -3    // Not enough money
#define SIG_ALI -2    // Already logged in
#define SIG_EXCEPTION -1
#define CMD_REGISTER 0
#define CMD_SIGNIN 1
#define CMD_JOIN 2
#define CMD_HISTORY 3
#define CMD_BID 4
#define CMD_SIGNOUT 5
#define CMD_END 6
#define CMD_WINNER 7
#define CMD_DEPOSIT 8
#define CMD_REJECT 9
#define STT_OFFLINE 0
#define STT_ONLINE 1
#define STT_BIDDING 2

typedef struct
{
    char name[50];
    int init_price;
    int curr_price;
    int min_incr;
} Goods;

typedef struct
{
    char name[40];
    char password[50];
    int balance;
    int status; // 0:offline, 1:online, 2:bidding
} User;

typedef struct
{
    int client;
    int bid_money;
} LogEntry;

// Administrating
void importDatabase();
void printGoodsList();
void ExportDatabase();
void addItem();
void editItem();
void deleteItem();
int selectAuctionItem();
int editDatabaseMenu();
void showAuctionHistory();
// void addHistory(char* username, char* goodsname, int bid);

// Utilities
int UserSignUp(User *user);
User *getUser(char *name);
char *fetchGoodsinfo();
char *getAuctionHistory(char *user_name);
void broadcast(char *str);
void broadcastEnd(int winner);
int UserAuthentication(User *user);
int checkLogIn(char *name);
int updateUserDetails(User user);
int writeAuctionLog(char *name, char *goods, int price, char *datetime);
char *modifyStr(char *str, char old, char new);
int deposit(User user, char *serial);
int choiceInMenu();
void *menuThread(void *threadid);
