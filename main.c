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
#include <stdbool.h>

#include "card_structs.h"
#include "card_funcs.h"

// *** DEFINES ***
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
    "Hearts ", "Clubs  ", "Diamond", "Spades "
};

const char rank_symbols[NUM_RANKS][3] =
{
    " A", " 2", " 3", " 4", " 5", " 6", " 7",
    " 8", " 9", "10", " J", " Q", " K"
};

const char rank_names[NUM_RANKS][6] =
{
    "  Ace", "  Two", "Three", " Four", " Five", "  Six", "Seven",
    "Eight", " Nine", "  Ten", " Jack", "Queen", " King"
};

// *** TYPEDEFS ***
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

typedef struct GameData
{
    RoundOutcome round_outcome;
    uint16_t cash;
    uint16_t pot;
    CardList deck;
    CardList player_hand;
    CardList dealer_hand;
} GameData;

typedef struct TextStagger_Uniform
{
    size_t count;
    uint32_t delay;
    char* chunks[];
} TextStagger_Uniform;

typedef struct TextStagger_VariableChunk
{
    uint16_t delay;
    char* text;
} TextStagger_VariableChunk;

// *** FUNCTION DECLARATIONS ***
// one-time game data initialization (dynamic for the test requirements)
GameData initialize_data(void);
// game intro message & prompt
void intro_sequence(void);
// blackjack outer loop (bet/quit)
void pregame(GameData* gameData);
// once per round initialization code
void initialize_round(GameData* gameData);
// blackjack core loop.
// considered extracting hit & stand into separate functions,
// but concluded that it would only increase complexity.
void game_loop(GameData* gameData);
// handle outcome, return 0 if no outcome & 1 if round over
bool handle_outcome(GameData *gameData);
// prints the contents of a card list.
// Note: this could have been split into 3-4 small "clean" functions,
// but I chose to determine suit, rank & value as well as
// print them all in one function, to avoid unnecessary
// code duplication, keeping it simple & fast.
int8_t show_hand(CardList *hand, uint16_t stagger, bool showAll);
// clears the screen & prints the game's "header" text
void new_frame(uint16_t stagger);
// prints the game's "footer" text
void footer(uint16_t stagger);
// waits for specified number of milliseconds
void delay_ms(uint32_t ms);
// empties stdin to avoid input shenanigans
void empty_stdin(void);
// repeatedly flashes text on screen for desired duration
void flash_text(uint8_t reps, uint32_t delay, const char *text);
// print out text chunks in uniform delay
void stagger_text_uniform(TextStagger_Uniform *pattern);
// same but with one repeated string
void stagger_text_uniform_repeat(size_t count, uint32_t delay, char* text);
// print out text chunks in variable delay
void stagger_text_variable(size_t count, const TextStagger_VariableChunk pattern[]);
// same but with one repeated string
void stagger_text_variable_repeat(size_t count, const uint16_t delay[], char* text);
// print out the characters of a single string in uniform delay
void stagger_string(uint16_t delay, const char* text);

/// *** FUNCTION DEFINITIONS ***
int main(int argc, char *argv[])
{
    // set mode if argument exists
    bool debugMode = (argc > 1) && (strcmp("debug", argv[1]) == 0);

    // initializing game state data
    GameData gameData;
    gameData = initialize_data();

    // initializing random seed
    srand(time(NULL));

    intro_sequence();

    // DEBUG: print initial contents of entire deck
    if (debugMode)
    {
        show_hand(&gameData.deck, 0, true);
        getchar();
    }

    // game outer loop (pregame <-> round).
    // considered reimplementing with an array of function pointers;
    // decided against it, but it was close.
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

void intro_sequence(void)
{
    new_frame(8);
    printf("Welcome to Blackjack!\n");
    footer(4);
    printf("Press 'Enter' to continue.\n");
    empty_stdin();
}

void pregame(GameData* gameData)
{
    int inputIsValid = 0;
    char answer = 'x';
    uint16_t bet = 0;
    gameData->round_outcome = UNDECIDED;

    new_frame(0);
    printf("===       BETTING       ===\n\n");
    printf("You have $%u in cash,\nand the pot is $%u.\n", gameData->cash, gameData->pot);
    footer(5);

    // no cash + no pot == no game
    if (gameData->cash < 10 && gameData->pot == 0)
    {
        delay_ms(200);
        gameData->round_outcome = BROKE;
        return;
    }

    printf("Play a round? (Y/N)\n");
    inputIsValid = scanf(" %c", &answer);
    empty_stdin();

    while (inputIsValid == 0 || (answer != 'Y' && answer != 'N' && answer!='y' && answer !='n'))
    {
        printf("Invalid answer, try again.\n");
        footer(0);
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

    new_frame(0);
    printf("-==-===  NEW ROUND  ===-==-\n\nPlayer initial hand:\n");
    playerValue = show_hand(&gameData->player_hand, 100, 1);
    printf("\n");
    delay_ms(100);

    if (playerValue == 21)
    // if exactly 21 player wins
    {
        gameData->round_outcome = PLAYER_BLACKJACK;
        return;
    }

    printf("Dealer initial hand:\n");
    show_hand(&gameData->dealer_hand, 150, 0);
    footer(4);
}

void game_loop(GameData* gameData)
{
    bool newPhase = true;
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
            new_frame(0);
            stagger_string(newPhase ? 5 : 0, "===         HIT         ===\n\n");
            flash_text(3, 300, "Dealing card to player!");
            MOVE_CARD(&gameData->deck, &gameData->player_hand, pick);
            delay_ms(50);

            // total value is recalculated
            stagger_string(10, "\n\nPlayer hand:\n");
            playerValue = show_hand(&gameData->player_hand, 250, 1);
            printf("\n");
            delay_ms(250);

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

            // else, dealer hand is reprinted,
            // and loop restarts
            stagger_string(0, "Dealer hand:\n");
            dealerValue = show_hand(&gameData->dealer_hand, 50, 0);
            footer(4);

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

        newPhase = false;
    }

    // dealer draws 
    //until their total value is 17 or over
    newPhase = true;

    for(;;)
    {
        new_frame(0);
        stagger_string(newPhase ? 5 : 0, "===    DEALER   DRAW    ===\n\nPlayer hand:\n");
        playerValue = show_hand(&gameData->player_hand, 0, 1);
        delay_ms(newPhase ? 20 : 200);

        stagger_string(newPhase ? 0 : 10, "\nDealer hand:\n");
        dealerValue = show_hand(&gameData->dealer_hand, newPhase ? 100 : 400, 1);
        footer(9);

        if (dealerValue >= 17 || dealerValue > playerValue) break;

        static const char* dealer_draw_text = "Dealer draws a card!";
        stagger_string(10, dealer_draw_text);
        stagger_string(10, "\r                    ");
        flash_text(3, 350, dealer_draw_text);
        delay_ms(50);
        pick = rand() % gameData->deck.length;
        MOVE_CARD(&gameData->deck, &gameData->dealer_hand, pick);
        newPhase = false;
    }

    // if it's over 21, player wins
    if (dealerValue > 21)
    {
        static const char* dealer_bust_text = "Dealer bust!";
        stagger_string(10, dealer_bust_text);
        stagger_string(20, "\r            \r");
        stagger_string(30, dealer_bust_text);
        stagger_string(10, "\r            \r");
        flash_text(2, 300, dealer_bust_text);
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
    static const char *blackjack_text = "BLACKJACK";
    static const TextStagger_VariableChunk tsvc_broke[3] =
    {   
        {800, "Out of gambling money."},
        {1000, "\a\n\n======  GAME"},
        {200, "\a OVER  ======\n\n"}
    };
    static const TextStagger_VariableChunk tsvc_quit[2] =
    {   
        {1000, "Enough Blackjack for now.\n"},
        {200, "\aDon't forget to gamble responsibly!\n"},
    };
    static const TextStagger_VariableChunk tsvc_player_win[6] =
    {   
        {800, "\aYou"},
        {250, "\a win"},
        {150, "\a this"},
        {800, "\a one,"},
        {200, "\a hu"},
        {200, "\aman!\n"}
    };

    switch (gameData->round_outcome)
    {
        case BROKE:
            stagger_text_variable(3, tsvc_broke);
            footer(7);
            return 1;
        case QUIT:
            new_frame(0);
            stagger_text_variable(2, tsvc_quit);
            footer(5);
            return 1;
        case UNDECIDED:
            return 0;
        case PLAYER_BLACKJACK:
            winning = gameData->pot * 2.5f;
            gameData->cash += winning;
            gameData->pot = 0;
            stagger_string(10, blackjack_text);
            stagger_string(20, "\r         \r");
            stagger_string(30, blackjack_text);
            stagger_string(20, "\r         \r");
            stagger_string(10, blackjack_text);
            stagger_text_uniform_repeat(8, 150, " !\a");
            printf("\nYou won $%u.\n", winning);
            break;
        case PLAYER_WIN:
            winning = gameData->pot * 2;
            gameData->cash += winning;
            gameData->pot = 0;
            stagger_text_variable(6, tsvc_player_win);
            printf("You won $%u.\n", winning);
            delay_ms(100);
            break;
        case PLAYER_LOSE:
            printf("\aToo bad, you lost.\n");
            delay_ms(200);
            stagger_string(20, "\aBetter luck next time.\n");
            gameData->pot = 0;
            break;
        case TIE:
            printf("\aIt's a tie!");
            stagger_string(30, " Money's still on the table...\n");
            break;
        default:
            printf("Unhandled outcome value: %d", gameData->round_outcome);
            break;
    }


    if (gameData->round_outcome > 0)
    {
        printf("\n-♥♣♦♠   ROUND  OVER  ♠♦♣♥-\n\n");
    }

    printf("Press 'Enter' to continue.\n");
    empty_stdin();

    return 1;
}

int8_t show_hand(CardList *hand, uint16_t stagger, bool showAll)
{
    uint16_t total = 0;
    uint8_t aces = 0;
    uint16_t count = 0;

    Card *current = hand->head;

    while(current != NULL)
    {
        delay_ms(stagger + count + total * (current->next == NULL ? 2 : 1));

        uint8_t rank = current->data >> 4;
        uint8_t suit_byte = (uint8_t)(current->data << 4);
        uint8_t suit = 0;
        uint8_t value = rank+1;

        if (value > 10) value = 10;
        else if (value == 1) aces++;

        total += value;

        while(suit_byte > 16)
        {
            suit_byte /= 2;
            suit++;
        }

        if (showAll || count == 0)
        {
            printf(" [%s%s] %s of %s (%2d)\n", rank_symbols[rank], suit_symbols[suit], rank_names[rank], suit_names[suit], value);
        }
        else
        {
            printf(" [ ? ]  ?\??  of   ?\??   (?\?)\n");
        }

        current = current->next;
        count++;
    }

    // account for aces being able to be either 1 or 10 in value
    while(total < 13 && aces > 0)
    {
        total += 9;
        aces--;
    }

    if (showAll)
    {
        printf("Total: [%hu]\n", total);
    }
    else
    {
        printf("Total: [??]\n");
    }

    return total;
}

void new_frame(uint16_t stagger)
{
    #if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        system("clear");
    #endif
    #if defined(_WIN32) || defined(_WIN64)
        system("cls");
    #endif

    static const char *text = "=======  BLACKJACK  =======\n\n";

    if (stagger)
    {
        stagger_string(stagger, text);
    }
    else
    {
        printf("%s", text);
    }
}

void footer(uint16_t stagger)
{
    static const char *text = "\n-====♥♣♦♠♥♣♦♠♥♠♦♣♥♠♦♣♥====-\n\n";

    if (stagger)
    {
        stagger_string(stagger, text);
    }
    else
    {
        printf("%s", text);
    }
}

void delay_ms(uint32_t ms)
{
    if (ms <= 0) return;

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

void stagger_text_uniform(TextStagger_Uniform *pattern)
{
    for (size_t i = 0; i < pattern->count; i++)
    {
        printf("%s", pattern->chunks[i]);
        fflush(stdout);
        delay_ms(pattern->delay);
    }
}

void stagger_text_uniform_repeat(size_t count, uint32_t delay, char* text)
{
    for (size_t i = 0; i < count; i++)
    {
        printf("%s", text);
        fflush(stdout);
        delay_ms(delay);
    }
}

void stagger_text_variable(size_t count, const TextStagger_VariableChunk pattern[])
{
    for (size_t i = 0; i < count; i++)
    {
        printf("%s", pattern[i].text);
        fflush(stdout);
        delay_ms(pattern[i].delay);
    }
}

void stagger_text_variable_repeat(size_t count, const uint16_t delay[], char* text)
{
    for (size_t i = 0; i < count; i++)
    {
        printf("%s", text);
        fflush(stdout);
        delay_ms(delay[i]);
    }
}

void stagger_string(uint16_t delay, const char* text)
{
    if (delay == 0)
    {
        printf("%s", text);
        return;
    }

    size_t length = strlen(text);

    for (size_t i = 0; i < length; i++)
    {
        printf("%c", text[i]);
        fflush(stdout);
        delay_ms(delay);
    }
}
