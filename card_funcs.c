#include "card_funcs.h"

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
