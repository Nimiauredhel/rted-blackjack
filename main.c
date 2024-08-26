#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define NUM_RANKS (13)
#define NUM_SUITS (4)

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
    uint8_t currentDeckLength;
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
// move card from one list to another
void move_card(Card *src, Card *dst, uint8_t srcIndex);
// writes the contents of card hands
uint8_t show_hand(Card *hand, uint8_t showAll);
// clears the screen
void clear();
// empties stdin to avoid input shenanigans
void empty_stdin();

int main(void)
{
    srand(time(NULL));

    uint8_t quit = 0;
    GameData gameData;
    gameData = initialize_data();

    //clear();
    printf("Welcome to Blackjack!\n");

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
    printf("Initializing game data struct.\n");
    GameData gameData;
    gameData.deck = malloc(sizeof(Card) * numCards);
    gameData.currentDeckLength = numCards;
    gameData.cash = 1000;
    gameData.pot = 0;
    gameData.dealerHand = NULL;
    gameData.playerHand = NULL;
    Card *previous = NULL;
    Card *current = NULL;

    // ranks
    for (int rankIdx = 0; rankIdx < NUM_RANKS; rankIdx++)
    {
        // suits
        for (int suitIdx = 0; suitIdx < NUM_SUITS; suitIdx++)
        {
            previous = current;
            current = &gameData.deck[rankIdx+(NUM_RANKS*suitIdx)];
            // set data to rank number
            current->data = rankIdx;
            // shift it 4 bits to the left
            current->data <<= 4;
            // set one of the first four bits to represent suit
            current->data |= (1 << suitIdx);

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

    printf("You have %u in cash, and the pot is %u.\nPlay a round? (Y/N)\n", gameData->cash, gameData->pot);
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

    while (inputIsValid == 0 || bet > gameData->cash || bet + gameData->pot <= 0)
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
    uint8_t pick;

    // deal two cards to player hand
    for (int i = 0; i < 2; i++)
    {
        pick = rand() % gameData->currentDeckLength;
        move_card(gameData->deck, gameData->playerHand, pick);
        gameData->currentDeckLength--;
    }

    // deal two cards to dealer hand
    for (int i = 0; i < 2; i++)
    {
        pick = rand() % gameData->currentDeckLength;
        move_card(gameData->deck, gameData->dealerHand, pick);
        gameData->currentDeckLength--;
    }

    printf("Player hand:\n");
    show_hand(gameData->playerHand, 1);

    printf("Dealer hand:\n");
    show_hand(gameData->dealerHand, 0);
}

uint8_t game_loop(GameData* gameData)
{
    printf("Blackjack GLOOP\n");
    return 0;
}

void move_card(Card *src, Card *dst, uint8_t srcIndex)
{
    printf("Moving a card!\n");
    // find prev of target card
    for (int i = 0; i < srcIndex-1; i++)
    {
        src = src->next;
    }

    Card *prev = src;
    src = src->next;

    // detach target card from src
    prev->next = src->next;
    src->next = NULL;

    // attach to end of dst
    if (dst == NULL)
    {
        dst = src;
    }
    else
    {
        while (dst->next != NULL)
        {
            dst = dst->next;
        }

        dst->next = src;
    }
}

uint8_t show_hand(Card *hand, uint8_t showAll)
{
    uint8_t total = 0;
    uint8_t aces = 0;

    while(hand != NULL)
    {
        uint8_t rank = hand->data << 4;
        printf(" %s of %s ", ranks[rank], "????");
        uint8_t value = rank+1;
        if (value > 10) value = 10;
        if (value == 1) aces++;
        total += value;
        hand = hand->next;
    }

    printf("\n");

    // account for aces being able to be either 1 or 10 in value
    while(total < 13 && aces > 0)
    {
        total += 9;
        aces--;
    }

    printf("Total: %hu \n", total);
    return total;
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
