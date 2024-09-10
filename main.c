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
#include <wchar.h>

// *** DEFINES ***
#define false (0)
#define true (1)
#define NUM_RANKS (13)
#define NUM_SUITS (4)
#define MOVE_CARD(src, dst, srcIndex) cardlist_add(dst, cardlist_draw(src, srcIndex))

// *** CONSTANTS ***
const uint8_t numCards = NUM_RANKS*NUM_SUITS; // aka 52
const char *hit_string = "hit\n";
const char *stand_string = "stand\n";

const char suit_symbols[NUM_SUITS][4] =
{
    "♥", "♣", "♦", "♠"
};

const char suit_names[NUM_SUITS][8] =
{
    "Hearts", "Clubs", "Diamond", "Spades"
};

const char rank_names[NUM_RANKS][6] =
{
    "Ace", "Two", "Three", "Four", "Five", "Six", "Seven",
    "Eight", "Nine", "Ten", "Jack", "Queen", "King"
};

// *** TYPEDEFS ***
typedef uint8_t bool;

typedef enum RoundOutcome
{
    BROKE = -2,
    QUIT = -1,
    UNDECIDED = 0,
    PLAYER_BLACKJACK = 1, // player wins pot * 2.5
    PLAYER_WIN = 2, // player wins pot * 2
    PLAYER_LOSE = 3, // no win, pot reset to zero
    TIE = 4 // no win, pot not reset
} RoundOutcome;

typedef struct Card
{
    uint8_t data;
    struct Card *next;
} Card;

typedef struct CardList
{
    Card *head;
    Card *tail;
    size_t length;
} CardList;

typedef struct GameData
{
    RoundOutcome round_outcome;
    uint16_t cash;
    uint16_t pot;
    CardList deck;
    CardList player_hand;
    CardList dealer_hand;
} GameData;

// *** FUNCTION DECLARATIONS ***
// one-time game data initialization (dynamic for the test requirements)
GameData initialize_data(void);
// blackjack outer loop (bet/quit)
void pregame(GameData* gameData);
// once per round initialization code
void initialize_round(GameData* gameData);
// blackjack core loop
void game_loop(GameData* gameData);
// handle outcome, return 0 if no outcome & 1 if round over
bool handle_outcome(GameData *gameData);
// writes the contents of card hands
int8_t show_hand(CardList *hand, bool showAll);
// clears the screen
void new_frame(void);
// waits for specified number of milliseconds
void delay_ms(uint32_t ms);
// empties stdin to avoid input shenanigans
void empty_stdin(void);
// repeatedly flashes text on screen for desired duration
void flash_text(uint8_t reps, uint32_t delay, const char *text);
// ** CARD LIST FUNCTIONS **
// initializes an empty card list
void cardlist_init(CardList *list);
// attaches a given card to the tail of a card list
void cardlist_add(CardList *list, Card *newCard);
// detaches and returns the head of a card list
Card* cardlist_pop(CardList *list);
// detaches and returns the specified element of card list
Card* cardlist_draw(CardList *list, uint8_t element);
// deallocates all cards of a card list
void cardlist_free(CardList *list);

/// *** FUNCTION DEFINITIONS ***
int main(void)
{
    // initializing random seed
    srand(time(NULL));

    // initializing game state data
    GameData gameData;
    gameData = initialize_data();

    // game intro message & prompt
    new_frame();
    printf("Welcome to Blackjack!\nPress 'Enter' to continue.\n");
    empty_stdin();

    // game outer loop (pregame <-> round)
    while(gameData.round_outcome > -1)
    {
        pregame(&gameData);
        if (handle_outcome(&gameData)) continue;
        initialize_round(&gameData);
        if (handle_outcome(&gameData)) continue;

        // game inner loop (hit or stand -> dealer draw)
        game_loop(&gameData);
        handle_outcome(&gameData);
    }
    
    // free all dynamically allocated memory
    // to match project requirements;
    // in a real-world project I would have
    // allocated the deck statically, which
    // would be both safer and more performant.
    cardlist_free(&gameData.deck);
    cardlist_free(&gameData.player_hand);
    cardlist_free(&gameData.dealer_hand);

    return 0;
}

GameData initialize_data(void)
{
    GameData gameData;

    gameData.round_outcome = UNDECIDED;
    gameData.cash = 1000;
    gameData.pot = 0;

    cardlist_init(&gameData.deck);
    cardlist_init(&gameData.player_hand);
    cardlist_init(&gameData.dealer_hand);

    Card *current = NULL;

    // ranks
    for (int rankIdx = 0; rankIdx < NUM_RANKS; rankIdx++)
    {
        // suits
        for (int suitIdx = 0; suitIdx < NUM_SUITS; suitIdx++)
        {
            current = malloc(sizeof(Card));
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

void pregame(GameData* gameData)
{
    int inputIsValid = 0;
    char answer = 'x';
    uint16_t bet = 0;
    gameData->round_outcome = UNDECIDED;

    new_frame();
    printf("      ===     BETTING     ===\n\n");
    printf("You have $%u in cash, and the pot is $%u.\n", gameData->cash, gameData->pot);
    delay_ms(200);

    // no cash + no pot == no game
    if (gameData->cash < 10 && gameData->pot == 0)
    {
        delay_ms(800);
        gameData->round_outcome = BROKE;
        return;
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

    if (answer == 'n' || answer == 'N')
    {
        gameData->round_outcome = QUIT;
        return;
    }

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
}

void initialize_round(GameData* gameData)
{
    uint8_t cardToDraw;
    uint8_t playerValue;

    // if player/dealer hands are not empty,
    // move them back to the deck
    while (gameData->player_hand.length > 0)
    {
        MOVE_CARD(&gameData->player_hand, &gameData->deck, 0);
    }

    while (gameData->dealer_hand.length > 0)
    {
        MOVE_CARD(&gameData->dealer_hand, &gameData->deck, 0);
    }

    // deal two cards to player hand
    for (int i = 0; i < 2; i++)
    {
        cardToDraw = rand() % gameData->deck.length;
        MOVE_CARD(&gameData->deck, &gameData->player_hand, cardToDraw);
    }

    // deal two cards to dealer hand
    for (int i = 0; i < 2; i++)
    {
        cardToDraw = rand() % gameData->deck.length;
        MOVE_CARD(&gameData->deck, &gameData->dealer_hand, cardToDraw);
    }

    printf("\n");

    printf("Player initial hand:\n");
    playerValue = show_hand(&gameData->player_hand, 1);
    printf("\n");
    delay_ms(150);

    if (playerValue == 21)
    // if exactly 21 player wins
    {
        gameData->round_outcome = PLAYER_BLACKJACK;
        return;
    }

    printf("Dealer initial hand:\n");
    show_hand(&gameData->dealer_hand, 0);
    printf("\n");
    delay_ms(150);
}

void game_loop(GameData* gameData)
{
    uint8_t pick = 0;
    uint8_t playerValue = 0;
    uint8_t dealerValue = 0;
    char reset_string[10] = "\0\0\0\0\0\0\0\0\0\0";
    char input[10];

    // hit or stand loop
    for(;;)
    {
        // HIT or STAND
        strcpy(input, reset_string); 
        printf("Would you like to Hit or Stand?\n");
        printf("(Enter \"hit\" or \"stand\" to answer)\n");
        fgets(input, 10, stdin);

        if (strcmp(input, hit_string) == 0)
        {
            // HIT: player draws another card
            pick = rand() % gameData->deck.length;
            new_frame();
            printf("      ===       HIT       ===\n\n");
            printf("Dealing card to player!\n\n");
            MOVE_CARD(&gameData->deck, &gameData->player_hand, pick);

            // total value is recalculated
            printf("Player hand:\n");
            playerValue = show_hand(&gameData->player_hand, 1);
            printf("\n");

            printf("Dealer hand:\n");
            dealerValue = show_hand(&gameData->dealer_hand, 0);

            // if over 21 player loses
            if (playerValue > 21)
            {
                gameData->round_outcome = PLAYER_LOSE;
                return;
            }
            // if exactly 21 player wins
            else if (playerValue == 21)
            {
                gameData->round_outcome = PLAYER_BLACKJACK;
                return;
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
    new_frame();

    for(;;)
    {
        new_frame();
        printf("      ===  DEALER   DRAW  ===\n\nPlayer hand:\n");
        playerValue = show_hand(&gameData->player_hand, 1);
        printf("\nDealer hand:\n");
        dealerValue = show_hand(&gameData->dealer_hand, 1);
        delay_ms(300);

        if (dealerValue >= 17 || dealerValue > playerValue) break;

        flash_text(3, 350, "Dealer draws a card!");
        delay_ms(100);
        pick = rand() % gameData->deck.length;
        MOVE_CARD(&gameData->deck, &gameData->dealer_hand, pick);
    }

    // if it's over 21, player wins
    if (dealerValue > 21)
    {
        flash_text(3, 350, "Dealer bust!");
        delay_ms(100);
        printf("\n");
        gameData->round_outcome = PLAYER_WIN;
        return;
    }
    // else if more than player, player loses
    else if (dealerValue > playerValue)
    {
        gameData->round_outcome = PLAYER_LOSE;
        return;
    }
    // equals is tie, less than: player wins .. duh
    else if (dealerValue == playerValue)
    {
        gameData->round_outcome = TIE;
        return;
    }

    gameData->round_outcome = PLAYER_WIN;
}

bool handle_outcome(GameData *gameData)
{
    uint32_t winning = 0;

    if (gameData->round_outcome > 0)
    {
        printf("\n   ======  ROUND OVER  ======\n\n");
    }

    switch (gameData->round_outcome)
    {
        case BROKE:
            printf("Out of gambling money.");
            fflush(stdout);
            delay_ms(800);
            printf("\a\n\n     ======  GAME");
            fflush(stdout);
            delay_ms(1200);
            printf("\a OVER  ======\n\n");
            fflush(stdout);
            delay_ms(300);
            return 1;
        case QUIT:
            new_frame();
            printf("\aEnough Blackjack for now.\n");
            fflush(stdout);
            delay_ms(1200);
            printf("\aDon't forget to gamble responsibly!\n");
            return 1;
        case UNDECIDED:
            return 0;
        case PLAYER_BLACKJACK:
            winning = gameData->pot * 2.5f;
            gameData->cash += winning;
            gameData->pot = 0;
            printf("BLACKJACK\a");
            for (int i = 0; i < 8; i++)
            {
                delay_ms(250);
                printf(" !\a");
                fflush(stdout);
            }
            printf("\nYou won $%u.\n", winning);
            break;
        case PLAYER_WIN:
            winning = gameData->pot * 2;
            gameData->cash += winning;
            gameData->pot = 0;
            printf("\aYou win this one,");
            fflush(stdout);
            delay_ms(600);
            printf("\a human!\n");
            fflush(stdout);
            delay_ms(600);
            printf("\aYou won");
            fflush(stdout);
            delay_ms(600);
            printf("\a $%u.\n", winning);
            break;
        case PLAYER_LOSE:
            printf("\aToo bad, you lost.\n");
            fflush(stdout);
            delay_ms(1200);
            printf("\aBetter luck next time.\n");
            gameData->pot = 0;
            break;
        case TIE:
            printf("\aIt's a tie! Money's still on the table...\n");
            break;
        default:
            printf("Unhandled outcome value: %d", gameData->round_outcome);
            break;
    }

    printf("Press 'Enter' to continue.\n");
    empty_stdin();

    return 1;
}

int8_t show_hand(CardList *hand, bool showAll)
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
        uint8_t value = rank+1;

        if (value > 10) value = 10;
        else if (value == 1) aces++;

        total += value;

        while(suite_byte > 16)
        {
            suite_byte /= 2;
            suite++;
        }

        if (showAll || count == 0)
        {
            printf(" %2d%-3s %s of %s \n", value, suit_symbols[suite], rank_names[rank], suit_names[suite]);
        }
        else
        {
            printf("  ?? ???? of ????\n");
        }

        current = current->next;
        count++;

        delay_ms(50);
    }

    delay_ms(50);
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

void new_frame(void)
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

void flash_text(uint8_t reps, uint32_t delay, const char *text)
{
    uint32_t third = delay/3;
    int len = strlen(text);
    char blank[len+1];

    memset(blank, ' ', len);
    blank[len] = '\0';

    printf("\n");

    for (int i = 0; i < reps; i++)
    {
        printf("\r%s", blank);
        fflush(stdout);
        delay_ms(third);

        printf("\r%s", text);
        fflush(stdout);
        delay_ms(third*2);
    }
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
Card* cardlist_draw(CardList *list, uint8_t element)
{
    if (element == 0)
    {
        return cardlist_pop(list);
    }

    if (list->length == 0) return NULL;
    Card* prev = NULL;
    Card* out = list->head;
    for (uint8_t i = 0; i < element; i++)
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

void cardlist_free(CardList *list)
{
    while(list->length > 0)
    {
        free(cardlist_pop(list));
    }
}
