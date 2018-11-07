#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>

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
#define TAUREIL 6
#define BARD 7
#define SMAUG 8
#define RING 9

typedef struct {
    int in_game;
    int hand;
    int immunity;
} player;

void shuffle(int *);

void swap(int *, int, int);

void discard(player *, int, int, int const *, int *);

int play_turn(int const *, int *, player *, int);



int main() {
    int i;
    int game_over = 0;
    int deck[] = {ARK, BILBO, THORIN, KILI, KILI, GANDALF, GANDALF, LEGOLAS,
                  TAUREIL, BARD, BARD, SMAUG, SMAUG, SMAUG, SMAUG, SMAUG, RING};

    int deck_pos = 4;
    player player1 = {1, 0, 0};
    player player2 = {1, 0, 0};
    player player3 = {1, 0, 0};
    player player4 = {1, 0, 0};
    player p[4] = {player1, player2, player3, player4};

    printf("\nHello, World!\n");

    shuffle(deck);

    for (i = 0; i < 4; i++){
        p[i].hand = deck[i];
    }

    int *p_deck = &deck_pos;

    for (i = 0; !game_over; i ++){
        i = i % 4;
        game_over = play_turn(deck, p_deck, p, i);
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
    switch (card){
        default:
            printf("Error has occurred");
            return;
        case ARK:
            p[player].in_game = 0;
            // TODO: SEND LOST GAME RECEIPT
            return;
        case BILBO:
            // nothing
            return;
        case THORIN:
            // TODO: CHOOSE TARGET
            printf("LA");
            int target = 2;
            int store = p[player].hand;
            p[player].hand = p[target].hand;
            p[target].hand = store;
            // TODO: SEND RECEIPTS
            return;
        case KILI:
            // TODO: CHOOSE TARGET
            printf("LA");
            int target = 2;
            if (p[target].hand == ARK){
                p[target].in_game = 0;
            } else {
                p[target].hand = deck[*p_deck];
                (*p_deck)++;
            }
            return;
        case GANDALF:
            p[player].immunity = 1;
            return;
        case LEGOLAS:
            // TODO: COMPARE
            return;
        case TAUREIL:
            // Compare
            return;
        case BARD:
            // TODO: PEEK
            return;
        case SMAUG:
            // TODO: GUESS
            return;
        case RING:
            // nothing
            return;
    }
}

#define HAND 1

int play_turn(int const deck[], int *p_deck, player p[], int player){
    // Draw
    int draw = deck[*p_deck], card;
    (*p_deck) ++;
    if ((draw == BILBO && (p[player].hand == KILI || p[player].hand == THORIN))
    || (p[player].hand == BILBO && (draw == KILI || draw == THORIN))) {
        // TODO: FORCE DISCARD
    }
    // TODO: GET CHOICE
    int choice = HAND;

    if (choice == HAND){
        card = p[player].hand;
        p[player].hand = draw;
        discard(p, player, card);
    } else {
        discard(p, player, draw);
    }

    int in = 0, i;
    for (i = 0; i < 4; i++){
        in += p[i].in_game;
    }
    if (in < 2){
        return 1;
    } else {
        return 0;
    }

}