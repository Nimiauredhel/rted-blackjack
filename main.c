#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define NUM_RANKS 13
#define NUM_SUITS 4

const uint8_t numCards = NUM_RANKS*NUM_SUITS; // aka 52

const char ranks[NUM_RANKS][5] =
{
    "Ace", "Two", "Three", "Four", "Five", "Six", "Seven",
    "Eight", "Nine", "Ten", "Jack", "Queen", "King"
};

const char suits[NUM_SUITS][7] =
{
    "Hearts", "Clubs", "Diamond", "Spades"
};

typedef struct Card
{
    uint8_t data;
    struct Card *next;
} Card;

typedef struct GameData
{
    uint16_t cash;
    uint16_t pot;
    Card *deck;
    Card *playerHand;
    Card *dealerHand;
} GameData;

// one-time game data initialization (dynamic for the test requirements)
GameData initialize_data();
// blackjack outer loop (bet/quit)
uint8_t pregame(GameData* gameData);
// once per round initialization code
void initialize_round(GameData* gameData);
// blackjack core loop
uint8_t game_loop(GameData* gameData);
// clears the screen
void clear();
// empties stdin to avoid input shenanigans
void empty_stdin();

int main(void)
{
    GameData gameData;
    gameData = initialize_data();
    clear();
    printf("Welcome to Blackjack!\n");

    uint8_t quit = 0;

    quit = pregame(&gameData);

    while(quit == 0)
    {
        initialize_round(&gameData);
        game_loop(&gameData);
        quit = pregame(&gameData);
    }

    return 0;
}

GameData initialize_data()
{
    GameData gameData;
    gameData.deck = malloc(sizeof(Card) * numCards);
    gameData.cash = 1000;
    gameData.pot = 0;
    Card *previous = NULL;
    Card *current = NULL;

    // ranks
    for (int i = 0; i < NUM_RANKS; i++)
    {
        // suits
        for (int j = 0; j < NUM_SUITS; j++)
        {
            previous = current;
            current = &gameData.deck[i*(j+1)];
            // set data to rank number
            current->data = i+1;
            // shift it 4 bits to the left
            current->data <<= 4;
            // set one of the first four bits to represent suit
            current->data |= (1 << j);

            if (previous != NULL) previous->next = current;
        }
    }

    return gameData;
}

uint8_t pregame(GameData* gameData)
{
    int inputIsValid = 0;
    char answer = 'x';
    uint16_t bet = 0;

    printf("You have %u in cash, and the pot is %u.\nPlay a round? (Y/N)\n", gameData->cash, gameData.pot);
    inputIsValid = scanf(" %c", &answer);
    empty_stdin();

    while (inputIsValid == 0 || answer != 'Y' && answer != 'N' && answer!='y' && answer !='n')
    {
        printf("Invalid answer, try again.\n");
        inputIsValid = scanf(" %c", &answer);
        empty_stdin();
    }

    if (answer == 'n' || answer == 'N') return 1;

    printf("How much (in multiples of 10) would you like to add to the pot?\n10 X ");
    inputIsValid = scanf(" %hu", &bet);
    empty_stdin();
    bet *= 10;

    while (inputIsValid == 0 || bet > *cash || bet + *pot <= 0)
    {
        printf("Invalid amount. You may only bet the cash that you have,\nand the pot must be greater than zero.\n10 X ");
        inputIsValid = scanf(" %hu", &bet);
        empty_stdin();
        bet *= 10;
    }

    gameData->cash -= bet;
    gameData->pot += bet;

    return 0;
}

void initialize_round(GameData* gameData)
{
    printf("Blackjack RINIT\n");
}

uint8_t game_loop(GameData* gameData)
{
    printf("Blackjack GLOOP\n");
    return 0;
}

void clear()
{
    #if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        system("clear");
    #endif
    #if defined(_WIN32) || defined(_WIN64)
        system("cls");
    #endif
}

void empty_stdin (void)
{
    int c = getchar();
    while (c != '\n' && c != EOF) c = getchar();
}
