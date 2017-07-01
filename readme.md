## Data Structures in C
=========================

Robust and reliable implementations of common data structures. These were developed for use in my own projects, as I wanted a way to abstract the C language a bit to make more complex projects easier to work with. Each data structure's implementation should closely match their professionally-developed counterparts. Since each structure can be passed in any data type, **function pointers** are (sometimes) needed to be passed into the constructor for a given structure. Some functions are optional to define and can be passed as `NULL` (ex. *toString*, if you don't need to print elements) while others are mandatory (ex. HashTable's *hash* function). Below is a table illustrating the caveats of each structure.

|Data Structure|Uses|Sorted?|Functions Required|Thread Safe|
|-|:-:|:-:|-|:-:
|Vector| Random Access, Deque, Stack, Queue|On Demand<br>Ω(n * log(n))|**compare** (optional, used for *sort*, *remove*, *contains*)<br>**toString** (optional, used for *print*)|Yes
|LinkedList|Deque, Stack, Queue|On Demand<br>Θ(n * log(n))|**compare** (optional, used for *sort*)<br>**toString** (optional, used for *print*)|Yes
|HashTable|Map, Set|No|**hash** (mandatory)<br>**equals** (mandatory)<br>**toString** (optional, used for *print*)|Yes
|Dictionary|Map, Set|Yes|**compare** (mandatory)<br>**toString** (optional, used for *print*)|Yes



## Example Uses:

### Vector
```c
#include "C-DataStructures/Vector.h"

int main()
{
    Vector* const deck = Vector_new(&compare, &toString);

    vect_grow_to(deck, 52);		// Not needed as the Vector grows automatically.
    // Add 52 cards to the Deck.
    for (enum card_suit suit = HEARTS; suit <= CLUBS; suit++)
        for (enum card_face face = ACE; face <= KING; face++)
        {
            Card *c = card(face, suit);
            vect_push_back(deck, c);
        }

    vect_shuffle(deck); // Shuffle and deck cards out to the players.
    const int PLAYERS = 6, CARDS_PER_PLAYER = 7;
    for (int i = 0; i < PLAYERS; i++)
        for (int h = 0; h < CARDS_PER_PLAYER; h++)
        {
            Card *dealt = vect_front(deck);
            vect_pop_front(deck);
        }

    printf("There are %d cards left in the deck.\n", vect_size(deck));  // 10 Cards Left
    vect_print(deck);
    vect_sort(deck);                                                    // Sort the cards.
    vect_print(deck);
    vect_erase(deck, 5);                                                // Delete the 6th card.
    
    // Locate a given card.
    Card search_val = { FIVE, HEARTS };
    bool success = vect_remove(deck, &search_val);
    printf("%s%s%s\n", success ? "Removed" : "Couldn't locate", " the ", toString(&search_val));
    search_val.face = ACE;
    printf("The %s does %sexist in the deck.\n", toString(&search_val), vect_contains(deck, &search_val) ? "" : "not ");

    vect_destroy(deck);
    return 0;
}
```

### LinkedList
```c
#include "C-DataStructures/LinkedList.h"

int main()
{
    LinkedList *list = LinkedList_new(&compare, &toString);

    // Add 52 cards to the Deck.
    for (enum card_suit suit = HEARTS; suit <= CLUBS; suit++)
        for (enum card_face face = ACE; face <= KING; face++)
        {
            Card *c = card(face, suit);
            list_push_back(deck, c);
        }

    // Shuffle the Deck of cards.
    list_shuffle(list);
    list_print(list);

    // Iterate over the list, starting at index 0.
    list_Iterator *iter = list_iter(list, 0);
    while (list_iter_has_next(iter))
    {
        Card* c = list_iter_next(iter);
        // Remove any card that isn't an Ace, Jack, Queen, or King.
        if (c->face >= TWO && c->face <= TEN)
            list_iter_remove(iter);
    }
    list_iter_destroy(iter);

    list_sort(list);
    printf("There are %u amount of cards in the list.\n", list_size(list));
    list_print(list);
    
    list_destroy(list);
    return 0;
}
```

### HashTable
```c
#include "C-DataStructures/HashTable.h"

int main()
{
    HashTable *table = HashTable_new(&hash, &equals, &keyValueToString);

    table_grow(table, 52);  // Not needed, but welcomed if you know an ideal size.
    for (enum card_suit suit = HEARTS; suit <= CLUBS; suit++)
        for (enum card_face face = ACE; face <= KING; face++)
        {
            Card *key = card(face, suit);
            int *value = malloc(sizeof(int));
            *value = min(face + 1, 10); // Cards in blackjack are worth 1 to 10
            table_put(table, key, value);
        }

    printf("There are %u elements in the table.\n", table_size(table));     // prints 52

    // Removal.
    Card *c1 = card(ACE, SPADES);
    if (table_remove(table, c1))
        printf("%s was removed.\n", toString(c1));
    else printf("%s was not removed.\n", toString(c1));

    // Searching.
    if (!table_contains(table, c1))
        printf("%s no longer exists in the table.\n", toString(c1));
    Card *c2 = card(JACK, CLUBS);
    int *score = table_get(table, c2);
    printf("The %s is worth %d points.\n", toString(c2), *score);

    table_print(table);

    // Iterating
    int aces = 0;
    table_Iterator *iter = table_iter(table);
    while (table_iter_has_next(iter))
    {
        int *value;
        Card *key = table_iter_next(iter, &value);
        if (*value == 1) aces++;
    }
    table_iter_destroy(iter);
    printf("There are %d Aces left in the table.\n", aces);

    table_clear(table);
    printf("The table is now empty: %s.\n", table_empty(table) ? "true" : "false");

    // Deconstruct the table.
    table_destroy(table);
    return 0;
}
```

### Dictionary
```c
#include <stdio.h>
#include "C-DataStructures/Dictionary.h"

int str_compare(const void *p1, const void *p2) { return strcmp(*(char**)p1, *(char**)p2); }
char* str_toString(const void *p) { return *(char**)p; }
char* int_toString(const void *p)
{
    static char buffer[25];
    sprintf(buffer, "%d", *(int*)p);
    return buffer;
}
char* kv_toString(const void *k, const void *v)
{
    static char buffer[25];
    sprintf(buffer, "<%s,%s>", str_toString(k), int_toString(v));
    return buffer;
}

void main()
{
    /* Dictionary constructor. */
    Dictionary *dict = Dictionary_new(&str_compare, &kv_toString);

    /* Add elements to the Dictionary. */
    char *name1 = "Jake", *name2 = "Max", *name3 = "Mark", *name4 = "John", *name5 = "Adam";
    int age1 = 16, age2 = 31, age3 = 24, age4 = 56, age5 = 24;
    dict_put(dict, &name1, &age1);
    dict_put(dict, &name2, &age2);
    dict_put(dict, &name3, &age3);
    dict_put(dict, &name4, &age4);
    dict_put(dict, &name5, &age5);

    dict_print(dict);

    char *name = "John";
    if (dict_contains(dict, &name))
        printf("%s's age is %d.\n", name, *(int*)dict_get(dict, &name));

    dict_remove(dict, &name);
    printf("There are %u mappings inside the Dictionary.\n", dict_size(dict));

    /* Loop through all mappings inside the Dictionary. */
    dict_Iterator* const iter = dict_iter(dict, PRE_ORDER);
    while (dict_iter_has_next(iter))
    {
        void *value;
        void *key = dict_iter_next(iter, &value);
        if (*(int*)value == 24)
            printf("24 year old located: %s.\n", *(char**)key);
    }
    dict_iter_destroy(iter);
    
    /* Destroy the Dictionary when finished. */
    dict_destroy(dict);
}
```