## Data Structures in C
=========================

Robust and reliable implementations of common data structures. These were developed for use in my own projects, as I wanted a way to abstract the C language a bit to make more complex projects easier to work with. Each data structure's implementation should closely match their professionally-developed counterparts. Since each structure can be passed in any data type, **function pointers** are (sometimes) needed to be passed into the constructor for a given structure. Some functions are optional to define and can be passed as `NULL` (ex. *toString*, if you don't need to print elements) while others are mandatory (ex. HashTable's *hash* function). Below is a table illustrating the caveats of each structure.

|Data Structure|Uses|Sorted?|Functions Required|Thread Safe|
|-|:-:|:-:|-|:-:
|Vector| Random Access, Deque, Stack, Queue|On Demand<br>Ω(n * log(n))|**compare** (optional, used for *sort*, *remove*, *contains*)<br>**toString** (optional, used for *print*)|Yes
|LinkedList|Deque, Stack, Queue|On Demand<br>Θ(n * log(n))|**compare** (optional, used for *sort*)<br>**toString** (optional, used for *print*)|No
|HashTable|Map, Set|No|**hash** (mandatory)<br>**equals** (mandatory)<br>**toString** (optional, used for *print*)|No



## Example Uses:

<details> 
  <summary>See Here for Card Implementation</summary>
   https://gist.github.com/KevinTyrrell/ae214b03e8a2052438491bdb3ff476fd
</details>

### Vector
```c
int main()
{
    Vector* const deck = Vector_new(&compare, &toString);

    // Not needed as the Vector grows automatically.
    vect_grow_to(deck, 52);
    // Add 52 cards to the Deck.
    for (int s = HEARTS; s <= CLUBS; s++)
        for (int f = ACE; f <= KING; f++)
        {
            Card *c = malloc(sizeof(Card));
            c->f = f;
            c->s = s;
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
    Card search_val = { HEARTS, FIVE };
    bool success = vect_remove(deck, &search_val);
    printf("%s%s%s\n", success ? "Removed" : "Couldn't locate", " the ", toString(&search_val));
    search_val.f = ACE;
    printf("The %s does %sexist in the deck.\n", toString(&search_val), vect_contains(deck, &search_val) ? "" : "not ");

    vect_destroy(deck);
    return 0;
}
```