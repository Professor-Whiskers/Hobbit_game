//
// Created by Theo Sellwood on 7/11/18.
//

/* Deck:
 *          Name                No. Points  ID  Effect
 *      - Arkenstone            1x  8       0   Lose Game
 *      - Bilbo Baggins         1x  7       1   Have to discard if with Thorin or Kili
 *      - Thorin Oakenshield    1x  6       2   Swap hands
 *      - Kili and bro          2x  5       3   Force Discard
 *      - Gandalf               2x  4       4   Protection
 *      - Legolas               1x  3       5   Compare, lower out
 *      - Taureil               1x  3       6   Compare, higher out
 *      - Bard                  2x  2       7   look at other player's card
 *      - Smaug                 5x  1       8   guess other player's card
 *      - The One Ring          1x  0/7     9   worth 7 at end of round
 */

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>


#define PORT 8080

#define ARK 0
#define BILBO 1
#define THORIN 2
#define KILI 3
#define GANDALF 4
#define LEGOLAS 5
#define TAURIEL 6
#define BARD 7
#define SMAUG 8
#define RING 9

int com_str(char const *, char const *, int);

void print_card(int);

int main(int argc, char const *argv[])
{
    int hand, draw = 0;
    int choice = 1;
    int over = 0;
    int sock = 0;
    int player = 0;
    int suc = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    read(sock, buffer, 128);
    printf("Your first card is ");
    hand = buffer[0];
    print_card(hand);
    //write(sock, "Ye", 3, 0);
    printf("\n");
    int c = 0;
    
    // BEGIN LOOP
    
    while (!over && c < 50){
        memset(buffer, 0, 1024);
        c ++;
        write(sock, &hand, 1);
        printf("\nListening...");
        fflush(stdout);
        read(sock, buffer, 5);
        printf("\n%s\n", buffer);
        if (com_str(buffer, "OVER", 4)){
            over = 1;
            printf("OVERED");
        } else
        if (com_str(buffer, "TURN", 4)){
            write(sock, &hand, 1);
            printf("\nIt's your turn.\n");
            read(sock, buffer, 5);
            printf("You drew ");
            draw = buffer[0];
            print_card(draw);
            printf("\n");
        } else
        if (com_str(buffer, "WAIT", 4)){
            printf("\nIt's another player's turn.");
        } else
        if (com_str(buffer, "BILD", 4)){
            printf("\nYou have to discard Bilbo.");
        } else
        if (com_str(buffer, "CHOD", 4)){
            printf("\nDo you want to play ");
            print_card(hand);
            printf(" (enter 1) or ");
            print_card(draw);
            printf(" (enter 0)\n");
            fflush(stdin);
            scanf("%d", &choice);
            printf("You chose %d\n", choice);
            suc = (int) write(sock, &choice, 1);
            printf("Code %d", suc);
            fflush(stdout);
        } else
        if (com_str(buffer, "1WIN", 4)){
            write(sock, &hand, 1);
            read(sock, buffer, 1);
            printf("GAME OVER. THE WINNER IS PLAYER %d", buffer[0]);
        } else
        if (com_str(buffer, "LOST", 4)){
            printf("You Lost.");
        } else
        if (com_str(buffer, "LOSS", 4)){
            write(sock, &hand, 1);
            read(sock, buffer, 1);
            printf("Player %d lost! Sucks to be them!", 1 + buffer[0]);
        } else
        if (com_str(buffer, "IDIS", 4)){
            write(sock, &hand, 1);
            read(sock, buffer, 1);
            printf("You discarded ");
            print_card(buffer[0]);
            printf("\n");
        } else
        if (com_str(buffer, "DISC", 4)){
            write(sock, &hand, 1);
            read(sock, buffer, 1);
            player = buffer[0];
            write(sock, &hand, 1);
            read(sock, buffer, 1);
            printf("Player %d discarded ", 1 + player);
            print_card(buffer[0]);
            printf("\n");
        } else
        if (com_str(buffer, "CHTP", 4)){

            suc = 1;
            while (suc){
                printf("Who do you want to target? ");
                fflush(stdin);
                scanf("%c", &buffer[0]);
                choice = (int) buffer[0] - '0';
                if (choice > 0 && choice < 5){
                    choice --;
                    suc = 0;
                } else {
                    printf("Invalid input.");
                }
            }
            printf("You chose player %d\n", choice);
            choice --;
            write(sock, &choice, 1);
        } else
        if (com_str(buffer, "CANT", 4)) {
            printf("Player cannot be targeted. ");
        } else
        if (com_str(buffer, "PLAC", 4)) {
            write(sock, &hand, 1);
            read(sock, buffer, 1);
            printf("Player %d looked at your hand.\n", buffer[0]);
        } else
        if (com_str(buffer, "BARD", 4)) {
            write(sock, &hand, 1);
            read(sock, buffer, 1);
            printf("Player %d has ", choice);
            print_card(buffer[0]);
            printf(".\n");
        }

    }
    return 0;
}

int com_str(char const *str1, char const *str2, int len){
    int i, b = 1;
    for (i = 0; i < len; i ++){
        b *= str1[i] == str2[i];
    }
    return b;
}

void print_card(int card){
    switch (card){
        default:
            printf("error\n");
            return;
        case ARK:
            printf("Arkenstone");
            return;
        case BILBO:
            printf("Bilbo Baggins");
            return;
        case THORIN:
            printf("Thorin Oakenshield");
            return;
        case KILI:
            printf("Kili the dwarf and Phili the dwarf");
            return;
        case GANDALF:
            printf("Gandalf the Grey");
            return;
        case LEGOLAS:
            printf("Legolas Greenleaf");
            return;
        case TAURIEL:
            printf("Tauriel");
            return;
        case BARD:
            printf("Bard the Bowman");
            return;
        case SMAUG:
            printf("Smaug");
            return;
        case RING:
            printf("The One Ring");
            return;
    }
}