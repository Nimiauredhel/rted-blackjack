#ifndef CARD_FUNCS_H
#define CARD_FUNCS_H

#include <stdlib.h>
#include "card_structs.h"

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

#endif
