<details>
  <summary>**Card implementation**</summary>
  <p>
##*Card.h*
```c

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

enum card_suit { HEARTS, SPADES, DIAMONDS, CLUBS };
enum card_face { ACE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING };
char* suit_strings[] = { "Hearts", "Spades", "Diamonds", "Clubs" };
char* face_strings[] = { "Ace", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King" };

typedef struct
{
    enum card_face face;
    enum card_suit suit;
} Card;

// Creates a Poker card.
Card* card(enum card_face face, enum card_suit suit)
{
    Card *c = malloc(sizeof(Card));
    if (c == NULL)
    {
        printf("Out of memory!\n");
        exit(1);
    }
    c->face = face;
    c->suit = suit;
    return c;
}

// Outputs the Card as a String.
char* toString(const void* v)
{
    static char buffer[25];
    const Card *var = v;
    sprintf(buffer, "%s%s%s", face_strings[var->face], " of ", suit_strings[var->suit]);
    return buffer;
}

char* keyValueToString(const void *v1, const void *v2)
{
    static char buffer[30];
    const int *value = v2;
    sprintf(buffer, "<%s,%d>", toString(v1), *value);
    return buffer;
}

// Compares two cards, returning -1, 0, or 1 depending on if a < b, a == b, or a > b
int compare(const Card *v1, const Card *v2)
{
    const Card *a = v1, *b = v2;
    if (a->suit != b->suit)
        return a->suit < b->suit ? -1 : 1;
    if (a->face != b->face)
        return a->face < b->face ? -1 : 1;
    return 0;
}

// Returns an integer representation of a Card.
unsigned int hash(const void *v)
{
    const Card *c = v;
    return (unsigned int)(10241 + 5124213 * c->face + c->suit);
}

// Check if two cards are equivalent.
bool equals(const void *v1, const void *v2)
{
    return compare(v1, v2) == 0;
}
```
</p></details>