#ifndef CARD_STRUCTS_H
#define CARD_STRUCTS_H

#include <stdio.h>
#include <stdint.h>

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

#endif
