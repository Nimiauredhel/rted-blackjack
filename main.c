#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// blackjack outer loop (bet/quit)
uint8_t pregame(uint16_t *cash, uint16_t *pot);
// once per round initialization code
void initialize_round();
// blackjack core loop
uint8_t game_loop();
// clears the screen
void clear();

int main(void)
{
    clear();
    printf("Welcome to Blackjack!\n");

    uint8_t quit = 0;
    uint16_t cash = 1000;
    uint16_t pot = 0;

    quit = pregame(&cash, &pot);

    while(quit == 0)
    {
        initialize_round();
        game_loop();
        quit = pregame(&cash, &pot);
    }

    return 0;
}
uint8_t pregame(uint16_t *cash, uint16_t *pot)
{
    char answer = 'x';
    uint16_t bet = 0;

    printf("You have %u in cash, and the pot is %u.\nPlay a round? (Y/N)\n", *cash, *pot);
    scanf(" %c", &answer);

    while (answer != 'Y' && answer != 'N' && answer!='y' && answer !='n')
    {
        printf("Invalid answer, try again.\n");
        scanf(" %c", &answer);
    }

    if (answer == 'n' || answer == 'N') return 1;

    do
    {
        printf("How much would you like to add to the pot?\n");
        scanf(" %hu", &bet);
    } while (bet > *cash || bet + *pot <= 0);

    *cash -= bet;
    *pot += bet;

    return 0;
}
void initialize_round()
{
    printf("Blackjack RINIT\n");
}
uint8_t game_loop()
{
    printf("Blackjack GLOOP\n");
    return 0;
}

void clear(){
    #if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        system("clear");
    #endif

    #if defined(_WIN32) || defined(_WIN64)
        system("cls");
    #endif
}
