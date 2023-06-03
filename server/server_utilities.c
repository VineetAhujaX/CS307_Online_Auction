#include "server_utilities.h"

extern Goods goods; // Goods to auction
extern Goods goodslist[100];
extern int good_count;    // Total number of goods in DB
extern int auction_state; // Default = 0, after first person join = 1,
                          // during 1st 20s of bid = 2,during 2nd 20s of bid = 3, during 3rd 20s of bid = 4
extern int isCount;
extern User *users;
extern LogEntry logging[500]; // auction logs

int UserSignUp(User *user)
{
    if (getUser(user->name) != NULL)
    {
        return 0;
    }
    FILE *file;
    file = fopen("users.txt", "a");
    if (file == NULL)
    {
        printf("Error in opening file!\n");
        return -1;
    }
    user->balance = 100000;
    fprintf(file, "%s\t%s\t%d\n", user->name, user->password, user->balance);
    fclose(file);
    return 1;
}

int updateUserDetails(User user)
{
    FILE *file;
    file = fopen("users.txt", "r");
    if (file == NULL)
    {
        printf("Error in opening file!\n");
        return 0;
    }
    User userList[100];
    int n = 0;
    int i;
    while (fscanf(file, "%s %s %d", userList[n].name, userList[n].password, &userList[n].balance) != EOF)
    {
        if (strcmp(user.name, userList[n].name) == 0)
        {
            userList[n].balance = user.balance;
        }
        n++;
    }
    fclose(file);
    file = fopen("users.txt", "w");
    if (file == NULL)
    {
        printf("Error in opening file!\n");
        return 0;
    }
    for (i = 0; i < n; i++)
    {
        fprintf(file, "%s\t%s\t%d\n", userList[i].name, userList[i].password, userList[i].balance);
    }
    fclose(file);
    return 1;
}

int UserAuthentication(User *user)
{
    User *userCheck;
    userCheck = getUser(user->name);
    if (userCheck == NULL)
        return 0;
    if (strcmp(user->password, userCheck->password) == 0)
    {
        user->balance = userCheck->balance;
        return 1;
    }
    return 0;
}

void broadcastEnd(int winner)
{
    int i;
    int cmd = CMD_END;
    char line[100];
    sprintf(line, "%d", cmd);
    for (i = 0; i < FD_SETSIZE; i++)
    {
        if (users[i].status == 2)
        {
            if (i != winner)
            {
                write(i, line, sizeof(char) * 100);
            }
            else
            {
                cmd = CMD_WINNER;
                sprintf(line, "%d", cmd);
                write(i, line, sizeof(char) * 100);
                cmd = CMD_END;
                sprintf(line, "%d", cmd);
            }
        }
    }
}

void broadcast(char *str)
{
    int i;
    for (i = 0; i < FD_SETSIZE; i++)
    {
        if (users[i].status == 2)
        {
            write(i, str, sizeof(char) * 100);
        }
    }
}

User *getUser(char *name)
{
    FILE *file;
    User *user;
    file = fopen("users.txt", "r");
    if (file == NULL)
    {
        printf("Error in opening file!\n");
        return NULL;
    }
    user = (User *)malloc(sizeof(User));
    while (fscanf(file, "%s %s %d", user->name, user->password, &user->balance) != EOF)
    {
        if (strcmp(user->name, name) == 0)
        {
            return user;
        }
    }
    fclose(file);
    return NULL;
}

int checkLogIn(char *name)
{
    int i;
    for (i = 0; i < FD_SETSIZE; i++)
    {
        if (strcmp(name, users[i].name) == 0)
            return 1;
    }
    return 0;
}

char *fetchGoodsinfo()
{
    char line[100];
    if (goods.curr_price == 0)
        sprintf(line, "\n  No auction is being held at the moment.\n");
    else
        sprintf(line, "  Goods: \t\t%s\n  Initial Price: \t%d INR\n  Current Price: \t%d INR\n  Minimum Increment: \t\t%d INR\n", goods.name, goods.init_price, goods.curr_price, goods.min_incr);
    line[strlen(line) - 1] = '\0';
    return strdup(line);
}

int choiceInMenu()
{
    int choice = 0;
    printf("**** AUCTION CONTROL MAIN MENU **** ");
    printf("\n\n");
    printf("1. Choose item for starting its auction.\n");
    printf("2. Edit item database.\n");
    printf("3. View auction history.\n");
    printf("4. Exit.\n");
    printf("\n Your choice (1-4): ");
    scanf("%d", &choice);
    while (!((choice >= 1) && (choice <= 4)))
    {
        printf("\n Invalid entry. Please input a number from 1 to 4 (options above): ");
        while (getchar() != '\n')
            ;
        scanf("%d", &choice);
    }
    return choice;
}

void importDatabase()
{
    FILE *file;
    int n = 0;
    Goods good;

    if ((file = fopen("goods.txt", "r")) == NULL)
    {
        printf("Error in opening Goods file!\n");
        return;
    }

    while (fscanf(file, "%d\t%d\t", &good.init_price, &good.min_incr) > 0)
    {
        fgets(good.name, 40, file);
        good.name[strlen(good.name) - 1] = '\0';
        goodslist[n].init_price = good.init_price;
        goodslist[n].min_incr = good.min_incr;
        strcpy(goodslist[n].name, good.name);
        n++;
    }
    fclose(file);
    good_count = n;
}

void exportDatabase()
{
    FILE *file;
    int i = 0;
    file = fopen("goods.txt", "w");
    if (file == NULL)
    {
        printf("Error in opening Goods file!\n");
        return;
    }

    for (i = 0; i < good_count; i++)
        fprintf(file, "%d\t%d\t%s\n", goodslist[i].init_price, goodslist[i].min_incr, goodslist[i].name);

    fclose(file);
}

int writeAuctionLog(char *name, char *goods, int price, char *datetime)
{
    FILE *file;
    file = fopen("log.txt", "a+");
    if (file == NULL)
    {
        printf("Error in opening Logs file!\n");
        return 0;
    }
    fprintf(file, "%s\t\t%s\t\t%d\t\t%s", name, goods, price, datetime);
    fclose(file);
    return 1;
}

// int deposit(User user,char* serial){
//     FILE *file;
//     char str[100];
//     int flag = 0;
//     int money = 0;
//     if((file = fopen("serial.txt","r"))==NULL)  {
//         printf("Error opening file!\n");
//         return 0;
//     }
//     while(fscanf(file,"%s %d",str,&money)!= EOF){
//         if(strcmp(serial,str)==0){
//             flag = 1;
//             break;
//         }
//     }
//     fclose(file);
//     if(!flag){
//         return 0;
//     }
//     user.balance += money;
//     updateUserDetails(user);
//     return money;
// }

void printGoodsList()
{
    int i = 0;
    printf("\n");
    printf("|-----|------------------------------------|---------------|---------------|\n");
    printf("| No. |              Item name             |  Start price  | Min increment |\n");
    printf("|-----|------------------------------------|---------------|---------------|\n");
    if (good_count < 1)
        printf("|                        There is no item in database !                    |\n");
    for (i = 0; i < good_count; i++)
        printf("| %3d | %-34s | %11d   | %11d   |\n", i + 1, goodslist[i].name, goodslist[i].init_price, goodslist[i].min_incr);
    printf("|-----|------------------------------------|---------------|---------------|\n");
}

int selectAuctionItem()
{
    int k = 0;
    printf("\n");
    if (good_count < 1)
    {
        printf("No items listed. Add items to the database first!\n");
        return 0;
    }
    printf("    Input the number corresponding to the item to auction: (1-%d): ", good_count);
    scanf("%d", &k);
    while ((k < 1) || (k > good_count))
    {
        printf("Invalid item number. Please input a number from 1 to %d : ", good_count);
        scanf("%d", &k);
    }
    goods.init_price = goodslist[k - 1].init_price;
    goods.min_incr = goodslist[k - 1].min_incr;
    goods.curr_price = goods.init_price;
    strcpy(goods.name, goodslist[k - 1].name);
    printf("Item- \"'%s'\" with start price- %d$ and minimum increment bid- %d$ SELECTED !\n", goods.name, goods.init_price, goods.min_incr);
    return 1;
}

int editDatabaseMenu()
{
    int choice = 0;
    printf("**** EDIT ITEM DATABSE **** ");
    printf("\n\n");
    printf("1. Add a new item.\n");
    printf("2. Edit pre-existing item.\n");
    printf("3. Delete an item.\n");
    printf("4. Back to main menu.\n");
    printf("\n Your choice (1-4): ");
    scanf("%d", &choice);
    while (!((choice >= 1) && (choice <= 4)))
    {
        printf("\n Invalid Entry. Please input a number from 1 to 4 (options above): ");
        while (getchar() != '\n')
            ;
        scanf("%d", &choice);
    }
    return choice;
}

void addItem()
{
    importDatabase();
    printf("** Enter details of the new item **\n\n");
    while (getchar() != '\n')
        ;
    printf("Name of the item: ");
    gets(goodslist[good_count].name);
    printf("Initial price: ");
    scanf("%d", &goodslist[good_count].init_price);
    printf("Minimum increment:");
    scanf("%d", &goodslist[good_count].min_incr);
    good_count++;
    exportDatabase();
    printf("\nItem added to database successfully !\n");
    printGoodsList();
}

void editItem()
{
    importDatabase();
    int choice = 0;
    printGoodsList();
    printf("\n\n");
    printf("**** Enter item number to edit ****");
    printf("\n\n");
    scanf("%d", &choice);
    while (!((choice >= 1) && (choice <= good_count)))
    {
        printf("\n Invalid Entry. Please input a number from 1 to %d (options above): ", good_count);
        scanf("%d", &choice);
    }
    printf("\n");
    while (getchar() != '\n')
        ;
    printf("New name of the goods: ");
    fgets(goodslist[choice - 1].name, 50, stdin);
    goodslist[choice - 1].name[strlen(goodslist[choice - 1].name) - 1] = '\0';
    printf("New initial price: ");
    scanf("%d", &goodslist[choice - 1].init_price);
    printf("New minimum increment:");
    scanf("%d", &goodslist[choice - 1].min_incr);

    exportDatabase();
    printf("\nItem edited successfully !\n");
    printGoodsList();
}

void deleteItem()
{
    importDatabase();
    int i = 0, choice = 0;
    printGoodsList();
    printf("\n\n");
    printf("**** Enter item number to delete ****");
    printf("\n\n");
    scanf("%d", &choice);
    while (!((choice >= 1) && (choice <= good_count)))
    {
        printf("\n Invalid Entry. Please input a number from 1 to %d (options above): ", good_count);
        scanf("%d", &choice);
    }
    printf("\n");
    good_count--;
    for (i = choice - 1; i < good_count; i++)
    {
        goodslist[i].init_price = goodslist[i + 1].init_price;
        goodslist[i].min_incr = goodslist[i + 1].min_incr;
        strcpy(goodslist[i].name, goodslist[i + 1].name);
    }

    exportDatabase();
    printf("\nItem deleted successfully !\n");
    printGoodsList();
}

char *getAuctionHistory(char *user_name)
{
    char history[1024];
    char line[100];
    char name[40];
    char goods_name[50];
    char price[10];
    char *datetime = (char *)malloc(sizeof(char) * 30);
    FILE *file;
    file = fopen("log.txt", "r");
    if (file == NULL)
    {
        printf("Error in opening Logs file!\n");
        return NULL;
    }
    sprintf(history, "\rGoods                         \tPrice     \tDate&Time\n");
    while (fscanf(file, "%s %s %s %s", name, goods_name, price, datetime) != EOF)
    {
        if (strcmp(name, user_name) == 0)
        {
            datetime = modifyStr(datetime, '.', ' ');
            strcat(price, " INR");
            sprintf(line, "%-30s\t%-10s\t%s\n", goods_name, price, datetime);
            strcat(history, line);
        }
    }
    fclose(file);
    return strdup(history);
}

char *modifyStr(char *str, char old, char new)
{
    int n = strlen(str);
    int i;
    char *str1 = strdup(str);
    for (i = 0; i < n; i++)
    {
        if (str1[i] == old)
        {
            str1[i] = new;
        }
    }
    return str1;
}

void showAuctionHistory()
{
    FILE *file;
    char line[100];
    file = fopen("log.txt", "r");
    if (file == NULL)
    {
        printf("Error opening auction history log file!\n");
        return;
    }

    printf("\n\n");
    printf("**** Auction History ****");
    printf("\n\n");

    while (fgets(line, 40, file) > 0)
    {
        printf("%s", line);
    }
    printf("\n\n");
    fclose(file);
}

/*
void addHistory(char* username, char* goodsname, int bid)
{
    time_t current_time;
    char* c_time_string;
    FILE *fo;
    FILE *fo;


    if((fo = fopen("auctionhistory.txt","a+"))==NULL)  {
        printf("Error opening auction history file!\n");
        return;
    }

    current_time = time(NULL);

    c_time_string = ctime(&current_time);
    if (c_time_string == NULL)
       fprintf(fo,"[Unknown time] User '%s' won '%s' with %d$.\n",username,goodsname,bid);
    else
        {
            c_time_string[strlen(c_time_string)-1] = '\0';
            fprintf(fo,"[%s] User '%s' won '%s' with %d$.\n",c_time_string,username,goodsname,bid);
        }
    fclose(fo);
}

*/

void *menuThread(void *threadid)
{
    int choice = 0;

main_menu:
    choice = choiceInMenu();
    switch (choice)
    {
    case 1: // Choose item, then start auction
        importDatabase();
        printGoodsList();
        if (selectAuctionItem() == 0)
            goto main_menu;
        do
        {
            printf("\n***Press \"Enter\" to immediately begin auction, or input 'b' to go back to menu***\n");
            while (getchar() != '\n')
                ;
            choice = getchar();
            if (choice == 'b')
                goto main_menu;
        } while (choice != '\n');
        isCount = 1;
        break;
    case 2: // Edit item database
    edit_menu:
        choice = editDatabaseMenu();
        switch (choice)
        {
        case 1:
            addItem();
            goto edit_menu;
            break;
        case 2:
            editItem();
            goto edit_menu;
            break;
        case 3:
            deleteItem();
            goto edit_menu;
            break;
        case 4:
            goto main_menu;
        }
        break;
    case 3: // View auction history
        showAuctionHistory();
        goto main_menu;
        break;
    case 4: // Exit
        exit(0);
        break;
    }
    return NULL;
}
