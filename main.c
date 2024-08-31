    #if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#define _GNU_SOURCE
    #endif
    #if defined(_WIN32) || defined(_WIN64)
    #endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_RANKS (13)
#define NUM_SUITS (4)
#define MOVE_CARD(src, dst, srcIndex) cardlist_add(dst, cardlist_draw(src, srcIndex))

const uint8_t numCards = NUM_RANKS*NUM_SUITS; // aka 52
const char *hit_string = "hit\n";
const char *stand_string = "stand\n";

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

typedef enum outcome_t
{
    BROKE = -2,
    QUIT = -1,
    NONE = 0,
    PLAYER_BLACKJACK = 1, // player wins pot * 2.5
    PLAYER_WIN = 2, // player wins pot * 2
    PLAYER_LOSE = 3, // no win, pot reset to zero
    TIE = 4 // no win, pot not reset
} outcome_t;

// one-time game data initialization (dynamic for the test requirements)
GameData initialize_data(void);
// blackjack outer loop (bet/quit)
outcome_t pregame(GameData* gameData);
// once per round initialization code
outcome_t initialize_round(GameData* gameData);
// blackjack core loop
outcome_t game_loop(GameData* gameData);
// handle outcome, return 0 if no outcome & 1 if round over
uint8_t handle_outcome(outcome_t outcome, GameData *gameData);
// writes the contents of card hands
int8_t show_hand(CardList *hand, uint8_t showAll);
// clears the screen
void clear(void);
// waits for specified number of milliseconds
void delay_ms(uint32_t ms);
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

    outcome_t outcome = 0;
    GameData gameData;
    gameData = initialize_data();

    clear();
    printf("Welcome to Blackjack!\nPress 'Enter' to continue.\n");
    empty_stdin();

    while(outcome > -1)
    {
        outcome = pregame(&gameData);
        if (handle_outcome(outcome, &gameData)) continue;
        outcome = initialize_round(&gameData);
        if (handle_outcome(outcome, &gameData)) continue;
        outcome = game_loop(&gameData);
        handle_outcome(outcome, &gameData);
    }

    free(gameData.cards);

    return 0;
}

GameData initialize_data(void)
{
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

    return gameData;
}

outcome_t pregame(GameData* gameData)
{
    int inputIsValid = 0;
    char answer = 'x';
    uint16_t bet = 0;

    clear();
    printf("      ===     BETTING     ===\n");
    printf("You have $%u in cash, and the pot is $%u.\n", gameData->cash, gameData->pot);
    delay_ms(500);

    if (gameData->cash < 10 && gameData->pot == 0)
    {
        delay_ms(1000);
         return -2;
    }
    printf("Play a round? (Y/N)\n");
    inputIsValid = scanf(" %c", &answer);
    empty_stdin();

    while (inputIsValid == 0 || (answer != 'Y' && answer != 'N' && answer!='y' && answer !='n'))
    {
        printf("Invalid answer, try again.\n");
        inputIsValid = scanf(" %c", &answer);
        empty_stdin();
    }

    if (answer == 'n' || answer == 'N') return -1;

    printf("How much (in multiples of 10) would you like to add to the pot?\n10 X $");
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

outcome_t initialize_round(GameData* gameData)
{
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
        MOVE_CARD(&gameData->deck, &gameData->playerHand, pick);
    }

    // deal two cards to dealer hand
    for (int i = 0; i < 2; i++)
    {
        pick = rand() % gameData->deck.length;
        MOVE_CARD(&gameData->deck, &gameData->dealerHand, pick);
    }

    printf("\n");

    printf("Player initial hand:\n");
    show_hand(&gameData->playerHand, 1);
    printf("\n");
    delay_ms(250);

    printf("Dealer initial hand:\n");
    show_hand(&gameData->dealerHand, 0);
    printf("\n");
    delay_ms(250);

    return 0;
}

outcome_t game_loop(GameData* gameData)
{
    uint8_t loop_end = 0;
    uint8_t pick = 0;
    uint8_t value = 0;
    char input[10] = "\0\0\0\0\0\0\0\0\0\0";

    // hit or stand loop
    while (loop_end == 0)
    {
        // HIT or STAND
        strcpy(input, "\0\0\0\0\0\0\0\0\0\0"); 
        printf("Would you like to Hit or Stand?\n");
        printf("(Enter \"hit\" or \"stand\" to answer)\n");
        fgets(input, 10, stdin);

        if (strcmp(input, hit_string) == 0)
        {
            // HIT: player draws another card
            pick = rand() % gameData->deck.length;
            clear();
            printf("      ===       HIT       ===\n");
            printf("Dealing card to player!\n");
            MOVE_CARD(&gameData->deck, &gameData->playerHand, pick);
            // total value is recalculated
            printf("Player hand:\n");
            value = show_hand(&gameData->playerHand, 1);
            printf("Dealer hand:\n");
            show_hand(&gameData->dealerHand, 0);
            // if over 21 player loses
            if (value > 21)
            {
                return PLAYER_LOSE;
            }
            // if exactly 21 player wins
            else if (value == 21)
            {
                return PLAYER_BLACKJACK;
            }
            // else, loop restarts
        }
        else if (strcmp(input, stand_string) == 0)

        {
            // STAND: loop breaks and we continue
            // to DEALER DRAW
            break;
        }
        else
        {
            printf("Invalid input, please try again.\n");
        }
    }

    // dealer draws 
    //until their total value is 17 or over
    uint8_t dealerValue = 0;
    clear();

    while (dealerValue < 17)
    {
        clear();
        printf("      ===  DEALER   DRAW  ===\nPlayer hand:\n");
        show_hand(&gameData->playerHand, 1);
        printf("\n");

        pick = rand() % gameData->deck.length;
        printf("Dealing card to dealer!\n");
        delay_ms(500);
        MOVE_CARD(&gameData->deck, &gameData->dealerHand, pick);
        printf("Dealer hand:\n");
        dealerValue = show_hand(&gameData->dealerHand, 1);
        delay_ms(1000);
    }

    delay_ms(500);

    // if it's over 21, player wins
    if (dealerValue > 21)
    {
        return PLAYER_WIN;
    }
    // else if more than player, player loses
    else if (dealerValue > value)
    {
        return PLAYER_LOSE;
    }
    // equals is tie, less than: player wins .. duh
    else if (dealerValue == value)
    {
        return TIE;
    }

    return PLAYER_WIN;
}

uint8_t handle_outcome(outcome_t outcome, GameData *gameData)
{
    uint32_t winning = 0;

    switch (outcome)
    {
        case BROKE:
            clear();
            printf("Out of gambling money.\nGAME OVER.\n");
            return 1;
        case QUIT:
            clear();
            printf("Enough Blackjack for now.\nDon't forget to gamble responsibly!\n");
            return 1;
        case NONE:
            return 0;
        case PLAYER_BLACKJACK:
            winning = gameData->pot * 2.5f;
            gameData->cash += winning;
            gameData->pot = 0;
            printf("IIIIT'S A BLACKJACK! CONGRATS!\nYou won $%u.\n", winning);
            break;
        case PLAYER_WIN:
            winning = gameData->pot * 2;
            gameData->cash += winning;
            gameData->pot = 0;
            printf("You win this one, human!\nYou won $%u.\n", winning);
            break;
        case PLAYER_LOSE:
            printf("Too bad, you lost. Better luck next time.\n");
            gameData->pot = 0;
            break;
        case TIE:
            printf("It's a tie! Money's still on the table...\n");
            break;
        default:
            printf("Unhandled outcome value: %d", outcome);
            break;
    }

    printf("Press 'Enter' to continue.\n");
    empty_stdin();

    return 1;
}

int8_t show_hand(CardList *hand, uint8_t showAll)
{
    uint8_t total = 0;
    uint8_t aces = 0;
    uint8_t count = 0;

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

        if (showAll || count == 0)
        {
            printf(" %s of %s ", ranks[rank], suits[suite]);
            uint8_t value = rank+1;

            if (value > 10) value = 10;
            else if (value == 1) aces++;

            total += value;
        }
        else
        {
            printf(" %s of ???? ", ranks[rank]);
        }

        current = current->next;
        count++;

        delay_ms(100);
    }

    delay_ms(100);
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

    printf("    =======  BLACKJACK  =======\n\n");
}

void delay_ms(uint32_t ms)
{
    #if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        uint32_t s = ms / 1000;
        ms = ms % 1000;
        struct timespec req = { s, ms * 1000000 };
        struct timespec rem = { s, ms * 1000000 };
        nanosleep(&req, &rem);
    #endif
    #if defined(_WIN32) || defined(_WIN64)
        clock_t start_time = clock();
        while (clock() < start_time + ms);
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
