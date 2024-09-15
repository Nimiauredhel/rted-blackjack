#ifndef FANCY_TEXT_H
#define FANCY_TEXT_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "delay.h"

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
// textually increment up to a number from 0
void run_up_number_2d(uint16_t delay, uint32_t number, uint32_t dramaNumber);

#endif
