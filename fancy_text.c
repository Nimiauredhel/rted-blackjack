#include "fancy_text.h"

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

void run_up_number_2d(uint16_t delay, uint32_t number, uint32_t dramaNumber)
{
    float current = 0;
    float drama = 1.0;

    printf("  ");

    while (current <= number)
    {
        drama = 1.1 - (fabsf(dramaNumber-current)/dramaNumber);
        printf("\b\b%2u", (uint32_t)current);
        fflush(stdout);
        delay_ms(delay*drama);
        current++;
    }
}
