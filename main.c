#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#define PORT 8080

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

typedef struct {
    int in_game;
    int hand;
    int immunity;
    int sock;
} player;

void shuffle(int *);

void swap(int *, int, int);

void discard(player *, int, int, int const *, int *);

int play_turn(int const *, int *, player *, int);

int get_point(int);

void end_game(player *);

int setup_sock(struct sockaddr_in, int);

void print_card(int);

void lose(player *, int);

void discard_receipt(player *, int, int);

int choose_target(player *, int);

void send_message(player *, char const *);

int main() {
    int i;
    int game_over = 0;
    int deck[] = {ARK, BILBO, THORIN, KILI, KILI, GANDALF, GANDALF, LEGOLAS,
                  TAURIEL, BARD, BARD, SMAUG, SMAUG, SMAUG, SMAUG, SMAUG, RING};
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    int deck_pos = 4;
    player player1 = {1, 0, 0, setup_sock(address, server_fd)};
    player player2 = {1, 0, 0, setup_sock(address, server_fd)};
    player player3 = {1, 0, 0, setup_sock(address, server_fd)};
    player player4 = {1, 0, 0, setup_sock(address, server_fd)};
    player p[4] = {player1, player2, player3, player4};
    shuffle(deck);

    for (i = 0; i < 4; i++){
        p[i].hand = deck[i];
        print_card(deck[i]);
        printf("\n");
        write(p[i].sock, &deck[i], 1);
        //read(p[i].sock, buffer, 5);
        //printf("%s", buffer);
    }

    int *p_deck = &deck_pos;

    for (i = 0; !game_over; i ++){
        i = i % 4;
        game_over = play_turn(deck, p_deck, p, i);
    }
    printf("OVER");
    for (i = 0; i < 4; i ++){
        send_message(p, "OVER");
        close(p[i].sock);
    }
    return 0;
}

void shuffle(int deck[]){
    srandom((unsigned  int) time(NULL));
    int i, a, b, s;
    for (i = 0; i < 50; i++){
        a = (int) (random() % 16);
        b = (int) (random() % 16);
        if (a == b){
            b = b - 1 % 16;
        }
        s = deck[a];
        deck[a] = deck[b];
        deck[b] = s;
    }
}

void swap(int arr[], int a, int b){
    int s = arr[a];
    arr[a] = arr[b];
    arr[b] = s;
}

void discard(player * p, int player, int card, int const *deck, int *p_deck){
    int target, store, p_point, t_point, i, buffer[1024] = {0};
    memset(buffer, 0, 1024);
    switch (card){
        default:
            printf("\nError has occurred\n");
            return;
        case ARK:
            discard_receipt(p, player, ARK);
            lose(p, player);
            return;
        case BILBO:
            // nothing
            discard_receipt(p, player, BILBO);
            return;
        case THORIN:
            discard_receipt(p, player, THORIN);
            target = choose_target(p, player);
            store = p[player].hand;
            p[player].hand = p[target].hand;
            p[target].hand = store;
            // TODO: write RECEIPTS
            return;
        case KILI:
            discard_receipt(p, player, KILI);
            target = choose_target(p, player);
            if (p[target].hand == ARK){
                discard_receipt(p, target, ARK);
                lose(p, target);
            } else {
                p[target].hand = deck[*p_deck];

                (*p_deck)++;
            }
            return;
        case GANDALF:
            discard_receipt(p, player, GANDALF);
            p[player].immunity = 1;
            return;
        case LEGOLAS:
            discard_receipt(p, player, LEGOLAS);
            target = choose_target(p, player);
            t_point = get_point(p[target].hand);
            p_point = get_point(p[player].hand);
            if (t_point > p_point){
                lose(p, player);
            } else if (p_point > t_point) {
                lose(p, target);
            } else {
                send_message(p, "NOTH");
            }
            return;
        case TAURIEL:
            discard_receipt(p, player, TAURIEL);
            // Compare
            target = choose_target(p, player);
            t_point = get_point(p[target].hand);
            p_point = get_point(p[player].hand);
            if (t_point < p_point){
                lose(p, player);
            } else if (p_point < t_point) {
                lose(p, target);
            } else {
                send_message(p, "NOTH");
            }
            return;
        case BARD:
            discard_receipt(p, player, BARD);
            target = choose_target(p, player);
            printf("Got target");
            read(p[player].sock, buffer, 10);
            write(p[player].sock, "BARD", 5);
            read(p[player].sock, buffer, 10);
            write(p[player].sock, &p[target].hand, 5);
            read(p[target].sock, buffer, 10);
            write(p[target].sock, "PLAC", 5);
            read(p[target].sock, buffer, 10);
            write(p[target].sock, &player, 5);
            //TODO: write PLAYER TARGET CARD
            return;
        case SMAUG:
            discard_receipt(p, player, SMAUG);
            target = choose_target(p, player);
            printf("GOT TARGET");
            // TODO: CHOOSE GUESS AND CHECK
            return;
        case RING:
            discard_receipt(p, player, RING);
            // nothing
            return;
    }
}

#define HAND 1

int play_turn(int const deck[], int *p_deck, player p[], int player){
    int i, choice;
    char buffer[1024] = {0};
    if ((*p_deck) > 16){
        end_game(p);
        return 1;
    }

    if (!p[player].in_game){
        printf("Player out");
        return 0;
    }
    for (i = 0; i < 4; i++){
        if (i == player){
            read(p[i].sock, buffer, 10);
            write(p[i].sock, "TURN", 5);
            read(p[i].sock, buffer, 10);
        } else {
            read(p[i].sock, buffer, 10);
            write(p[i].sock, "WAIT", 5);
        }
    }
    p[player].immunity = 0;
    // Draw
    int draw = deck[*p_deck], card;
    (*p_deck) ++;

    printf("Player %d drew %d\n", 1+ player, draw);
    write(p[player].sock, &draw, 1);

    if ((draw == BILBO && (p[player].hand == KILI || p[player].hand == THORIN))
    || (p[player].hand == BILBO && (draw == KILI || draw == THORIN))) {
        read(p[player].sock, buffer, 10);
        write(p[player].sock, "BILD", 5);
        discard(p, player, BILBO, deck, p_deck);
        if (p[player].hand == BILBO){
            p[player].hand = draw;
        }
    } else {
        read(p[player].sock, buffer, 10);
        write(p[player].sock, "CHOD", 5);
        printf("asked\n");
        read(p[player].sock, buffer, 10);
        choice = buffer[0];
        printf("choose : %d\n", choice);
        if (choice){
            card = p[player].hand;
            p[player].hand = draw;
            discard(p, player, card, deck, p_deck);
            printf("Code 1.");
        } else {
            discard(p, player, draw, deck, p_deck);
        }
    }


    int in = 0, win = 0;
    for (i = 0; i < 4; i++){
        in += p[i].in_game;
        if (p[i].in_game){
            win = i;
        }
    }
    if (in < 2){
        printf("WINNER! Player %d", win + 1);
        return 1;
    } else {
        return 0;
    }

}

int get_point(int card){
    switch (card){
        default:
            return 7;
        case ARK:
            return 8;
        case BILBO:
            return 7;
        case THORIN:
            return 6;
        case KILI:
            return 5;
        case GANDALF:
            return 4;
        case LEGOLAS:
            return 3;
        case TAURIEL:
            return 3;
        case BARD:
            return 2;
        case SMAUG:
            return 1;
        case RING:
            return 0;
    }
}

void end_game(player p[]){
    int winner = 0, w_score = 0, score, i;
    for (i = 0; i < 4; i++){
        if (p[i].in_game){
            if(!(score = get_point(p[i].hand))){
                score = 7;
            }
            if (score > w_score){
                winner = i;
                w_score = score;
            }
        }
    }
    winner ++;
    printf("Player %d", winner);
    for (i = 0; i < 4; i ++){
        read(p[i].sock, &w_score, 1);
        write(p[i].sock, "1WIN", 5);
        read(p[i].sock, &w_score, 1);
        write(p[i].sock, &winner, 1);
    }
}

int setup_sock(struct sockaddr_in address, int server_fd){
    int addrlen = sizeof(address), new_socket;

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    return new_socket;
}


void print_card(int card){
    switch (card){
        default:
            printf("error");
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

void lose(player *p, int loser){
    int i, store;
    p[loser].in_game = 0;
    for (i = 0; i < 4; i++) {
        if (i == loser) {
            read(p[i].sock, &store, 10);
            write(p[i].sock, "LOST", 5);
        } else {
            read(p[i].sock, &store, 10);
            write(p[i].sock, "LOSS", 5);
            read(p[i].sock, &store, 10);
            write(p[i].sock, &loser, 1);
        }
    }
}

void discard_receipt(player *p, int player, int card){
    int i, store[1024] = {0};
    printf("Player %d discarded ", player + 1);
    print_card(card);
    printf("\n");
    for (i = 0; i < 4; i++) {
        if (i == player) {
            read(p[i].sock, &store, 10);
            write(p[i].sock, "IDIS", 5);
            read(p[i].sock, &store, 10);
            write(p[i].sock, &card, 1);
        } else {
            read(p[i].sock, &store, 10);
            write(p[i].sock, "DISC", 5);
            read(p[i].sock, &store, 10);
            write(p[i].sock, &player, 1);
            read(p[i].sock, &store, 10);
            write(p[i].sock, &card, 1);
        }
    }
}

int choose_target(player *p, int player){
    char buffer[128] = {0};
    int target;
    printf("Listening...");
    read(p[player].sock, buffer, 10);
    write(p[player].sock, "CHTP", 5);
    printf("asked %d target\n", player);
    read(p[player].sock, buffer, 10);
    target = buffer[0];
    printf("choose : %d\n", target);
    if (!p[target].in_game || p[target].immunity){
        read(p[player].sock, buffer, 10);
        write(p[player].sock, "CANT", 5);
        target = choose_target(p, player);
    }
    return target;
}

void send_message(player *p, char const *msg){
    int i;
    char buffer[10] = {0};
    for (i = 0; i < 4; i ++){
        read(p[i].sock, buffer, 10);
        write(p[i].sock, msg, 5);
    }
}