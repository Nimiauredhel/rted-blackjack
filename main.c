#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define NUM_RANKS (13)
#define NUM_SUITS (4)
#define MOVE_CARD(src, dst, srcIndex) cardlist_add(dst, cardlist_draw(src, srcIndex))

const uint8_t numCards = NUM_RANKS*NUM_SUITS; // aka 52

const char ranks[NUM_RANKS][6] =
{
    "Ace", "Two", "Three", "Four", "Five", "Six", "Seven",
    "Eight", "Nine", "Ten", "Jack", "Queen", "King"
};

const char suits[NUM_SUITS][8] =
{
    "Hearts", "Clubs", "Diamond", "Spades"
};

typedef struct Card
{
    uint8_t data;
    struct Card *next;
} Card;

typedef struct CardList
{
    Card *head;
    Card *tail;
    uint8_t length;
} CardList;

typedef struct GameData
{
    uint16_t cash;
    uint16_t pot;
    Card* cards;
    CardList deck;
    CardList playerHand;
    CardList dealerHand;
} GameData;

// one-time game data initialization (dynamic for the test requirements)
GameData initialize_data(void);
// blackjack outer loop (bet/quit)
uint8_t pregame(GameData* gameData);
// once per round initialization code
void initialize_round(GameData* gameData);
// blackjack core loop
uint8_t game_loop(GameData* gameData);
// writes the contents of card hands
uint8_t show_hand(CardList *hand, uint8_t showAll);
// clears the screen
void clear(void);
// empties stdin to avoid input shenanigans
void empty_stdin(void);
// *** card list functions ***
void cardlist_init(CardList *list);
void cardlist_add(CardList *list, Card *newCard);
Card* cardlist_pop(CardList *list);
Card* cardlist_draw(CardList *list, uint8_t index);

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

    free(gameData.cards);
    return 0;
}

GameData initialize_data(void)
{
    printf("Initializing game data struct.\n");
    GameData gameData;
    gameData.cards = malloc(sizeof(Card) * numCards);
    cardlist_init(&gameData.deck);
    cardlist_init(&gameData.playerHand);
    cardlist_init(&gameData.dealerHand);
    gameData.cash = 1000;
    gameData.pot = 0;

    Card *current = NULL;

    // ranks
    for (int rankIdx = 0; rankIdx < NUM_RANKS; rankIdx++)
    {
        // suits
        for (int suitIdx = 0; suitIdx < NUM_SUITS; suitIdx++)
        {
            current = &gameData.cards[rankIdx+(NUM_RANKS*suitIdx)];
            // set data to rank number
            current->data = rankIdx;
            // shift it 4 bits to the left
            current->data <<= 4;
            // set one of the first four bits to represent suit
            current->data |= (1 << suitIdx);

            cardlist_add(&gameData.deck, current);
        }
    }

    show_hand(&gameData.deck, 0);
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

    while (inputIsValid == 0 || (answer != 'Y' && answer != 'N' && answer!='y' && answer !='n'))
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

    // if player/dealer hands are not empty,
    // move them back to the deck
    while (gameData->playerHand.length > 0)
    {
        MOVE_CARD(&gameData->playerHand, &gameData->deck, 0);
    }

    while (gameData->dealerHand.length > 0)
    {
        MOVE_CARD(&gameData->dealerHand, &gameData->deck, 0);
    }

    // deal two cards to player hand
    for (int i = 0; i < 2; i++)
    {
        pick = rand() % gameData->deck.length;
        printf("Dealing player card %d!\n", i);
        MOVE_CARD(&gameData->deck, &gameData->playerHand, pick);
    }

    // deal two cards to dealer hand
    for (int i = 0; i < 2; i++)
    {
        pick = rand() % gameData->deck.length;
        printf("Dealing dealer card %d!\n", i);
        MOVE_CARD(&gameData->deck, &gameData->dealerHand, pick);
    }

    printf("Player hand:\n");
    show_hand(&gameData->playerHand, 1);

    printf("Dealer hand:\n");
    show_hand(&gameData->dealerHand, 0);
}

uint8_t game_loop(GameData* gameData)
{
    printf("Blackjack GLOOP\n");
    return 0;
}

uint8_t show_hand(CardList *hand, uint8_t showAll)
{
    uint8_t total = 0;
    uint8_t aces = 0;

    Card *current = hand->head;

    while(current != NULL)
    {
        uint8_t rank = current->data >> 4;
        uint8_t suite_byte = (uint8_t)(current->data << 4);
        uint8_t suite = 0;
        while(suite_byte > 16)
        {
            suite_byte /= 2;
            suite++;
        }
        printf(" %s of %s ", ranks[rank], suits[suite]);
        uint8_t value = rank+1;
        if (value > 10) value = 10;
        if (value == 1) aces++;
        total += value;
        current = current->next;
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

void clear(void)
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

void cardlist_init(CardList *list)
{
    list->length = 0;
    list->head = NULL;
    list->tail = NULL;
}
void cardlist_add(CardList *list, Card *newCard)
{
    if (list->length == 0)
    {
        list->head = newCard;
        list->tail = newCard;
        list->length = 1;
    }
    else
    {
        list->tail->next = newCard;
        list->tail = newCard;
        list->length++;
    }
}
Card* cardlist_pop(CardList *list)
{
    if (list->length == 0) return NULL;
    Card* out = list->head;
    list->length--;

    if (list->length == 0)
    {
        list->head = NULL;
        list->tail = NULL;
    }
    else
    {
        list->head = out->next;
    }

    out->next = NULL;
    return out;
}
Card* cardlist_draw(CardList *list, uint8_t index)
{
    if (index == 0)
    {
        return cardlist_pop(list);
    }

    if (list->length == 0) return NULL;
    Card* prev = NULL;
    Card* out = list->head;
    for (uint8_t i = 0; i < index; i++)
    {
        prev = out;
        out = out->next;
    }

    list->length--;

    if (list->length == 0)
    {
        list->head = NULL;
        list->tail = NULL;
    }
    else
    {
        if (out->next == NULL)
        {
            list->tail = prev;
        }

        prev->next = out->next;
    }

    out->next = NULL;

    return out;
}
