
/*
 * File: Dictionary.c
 * Date: Sep 01, 2016
 * Name: Kevin Tyrrell
 * Version: 4.0.0
 */

/*
Copyright © 2017 Kevin Tyrrell

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Dictionary.h"

/* Node Colors.
 * RED and LEFT must be false for Calloc initialization */
#define RED (bool)false
#define BLACK (bool)true
#define LEFT (bool)false
#define RIGHT (bool)true

/* Basic tree navigation. */
#define PARENT(node) node->parent
#define GRANDPARENT(node) PARENT(PARENT(node))
#define IS_ROOT(node) (PARENT(node) == NULL)
#define LEAF(node) (node->left == NULL && node->right == NULL)
#define DIRECTION(child, parent) (parent->left == child ? LEFT : RIGHT)
#define CHILD(node, direction) *(direction == LEFT ? &node->left : &node->right)
#define COLOR(node) (node == NULL || node->color == BLACK ? BLACK : RED)
#define SWAP_PTR(p1, p2) void *t = p1; p1 = p2; p2 = t

/* Node structure. */
typedef struct dict_Node
{
    const void *key, *value;
    struct dict_Node *left, *right, *parent;
    bool color;
} dict_Node;

/* Dictionary structure. */
struct Dictionary
{
    dict_Node *root;
    size_t size;

    /* Synchronization. */
    ReadWriteSync *rw_sync;

    /* Function pointers. */
    int(*compare)(const void*, const void*);
    char*(*toString)(const void*, const void*);
};

/* Structure to assist in looping through Dictionary. */
struct dict_Iterator
{
    const dict_Node *current;
    Vector *stack;

    /* Function pointer. */
    dict_Node*(*next)(dict_Iterator*);
};

/* Local functions. */
static dict_Node* dict_Node_new(const void* const key, const void* const value);
static dict_Node* dict_binary_search(const Dictionary* const dict, const void* const key, int* const compared);
static dict_Node* dict_Node_uncle(const dict_Node* const child);
static dict_Node* dict_iter_next_node(dict_Iterator* const iter);
static dict_Node* dict_iter_in_order(dict_Iterator* const iter);
static dict_Node* dict_iter_pre_order(dict_Iterator* const iter);
static dict_Node* dict_iter_post_order(dict_Iterator* const iter);
static unsigned int dict_Node_height(const dict_Node* const node);
static void dict_enforce_rb_properties(Dictionary* const dict, dict_Node *child);
static void dict_assign_child(dict_Node* const parent, dict_Node* const child, const bool direction);
static void dict_heapify(const dict_Node* const current, const dict_Node** const arr, const unsigned int index);
static void dict_Node_destroy(dict_Node* const node);

/*
 * Constructor function.
 * The `compare` function must be defined to call this function.
 * Θ(1)
 */
Dictionary* Dictionary_new(int(*compare)(const void*, const void*),
                           char*(*toString)(const void*, const void*))
{
    io_assert(compare != NULL, IO_MSG_NOT_SUPPORTED);

    Dictionary* const dict = mem_calloc(1, sizeof(Dictionary));
    dict->compare = compare;
    dict->toString = toString;
    dict->rw_sync = ReadWriteSync_new();
    return dict;
}

/*
 * Returns the value of an entry that corresponds to the specified key.
 * Returns NULL if the key does not exist in the Dictionary.
 * Θ(log(n))
 */
void* dict_get(const Dictionary* const dict, const void* const key)
{
    io_assert(dict != NULL, IO_MSG_NULL_PTR);
    io_assert(key != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future writers. */
    sync_read_start(dict->rw_sync);

    int compared;
    const dict_Node* const searched = dict_binary_search(dict, key, &compared);
    const void* const value = (searched != NULL && compared == 0) ? searched->value : NULL;

    /* Unlock the data structure. */
    sync_read_end(dict->rw_sync);

    return (void*)value;
}

/*
 * Returns the number of elements in the Dictionary.
 * Θ(1)
 */
size_t dict_size(const Dictionary* const dict)
{
    io_assert(dict != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future writers. */
    sync_read_start(dict->rw_sync);

    const size_t size = dict->size;

    /* Unlock the data structure. */
    sync_read_end(dict->rw_sync);

    return size;
}

/*
 * Returns true if the Dictionary is empty.
 * Θ(1)
 */
bool dict_empty(const Dictionary* const dict)
{
    io_assert(dict != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future writers. */
    sync_read_start(dict->rw_sync);

    const bool empty = dict->size == 0;

    /* Unlock the data structure. */
    sync_read_end(dict->rw_sync);

    return empty;
}

/*
 * Returns true if the Dictionary contains the specified key.
 * Θ(log(n))
 */
bool dict_contains(const Dictionary* const dict, const void* const key)
{
    io_assert(dict != NULL, IO_MSG_NULL_PTR);
    io_assert(key != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future writers. */
    sync_read_start(dict->rw_sync);

    int compared;
    const bool located = dict_binary_search(dict, key, &compared) != NULL && compared == 0;

    /* Unlock the data structure. */
    sync_read_end(dict->rw_sync);

    return located;
}

/*
 * Prints out the structure of the Dictionary to the console window.
 * Θ(n)
 */
void dict_print_tree(const Dictionary* const dict)
{
    io_assert(dict != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future writers. */
    sync_read_start(dict->rw_sync);

    if (dict->size > 0)
    {
        /* Parse the Dictionary into a heap. */
        const size_t height = dict_Node_height(dict->root),
                elements = (unsigned int)math_pow(2, height) - 1;
        const dict_Node** const heap = mem_calloc(elements, sizeof(dict_Node*));
        dict_heapify(dict->root, heap, 0);

        const size_t LETTERS = 3;

        /* Local macros to simplify spacing of the text-based Dictionary. */
        #define PRINT_SPACES(n) do { if (n > 0) printf("%*c", n, ' '); } while (false)
        #define LINE_SPACES_DATA(current_height) LETTERS * ((unsigned int)math_pow(2, height - current_height) - 1)
        #define LINE_SPACES_ARROWS(current_height) LETTERS * (unsigned int)math_pow(2, height - current_height - 1) - 1

        unsigned int index = 0;
        for (unsigned int iter_height = 1; iter_height <= height; iter_height++)
        {
            /* Print all of the data elements on this row of the Dictionary. */
            PRINT_SPACES(LINE_SPACES_DATA(iter_height));
            const unsigned int data_elements = (unsigned int)math_pow(2, iter_height - 1);
            for (unsigned int i = 0; i < data_elements; i++)
            {
                /* Print the data element and limit it's character count. */
                const dict_Node* const element = heap[index++];
                if (element != NULL)
                    printf ("%c%.*s", element->color == RED ? 'R' : 'B',
                            LETTERS - 1, dict->toString(element->key, element->value));
                else printf("...");

                /* Add spaces after the element for the next iteration or go to next line. */
                if (i + 1 < data_elements)
                    PRINT_SPACES(LINE_SPACES_DATA(iter_height + 1));
                else printf("\n");
            }

            /* Don't print arrows on the very last row. */
            if (iter_height >= height) break;
            /* Print all the lines that attach the Nodes. */
            PRINT_SPACES(LINE_SPACES_ARROWS(iter_height));
            for (unsigned int i = 0; i < data_elements; i++)
            {
                printf("%c", '/');
                PRINT_SPACES(LINE_SPACES_DATA(iter_height));
                printf("%c", '\\');

                /* Add spaces after the arrow for the next iteration or go to next line. */
                if (i + 1 < data_elements)
                    PRINT_SPACES(LINE_SPACES_DATA(iter_height) + LETTERS + 1);
                else printf("\n");
            }
        }

        mem_free(heap, elements * sizeof(dict_Node*));
    }

    /* Unlock the data structure. */
    sync_read_end(dict->rw_sync);
}

/*
 * Returns a shallow copy of the Dictionary.
 * Θ(n)
 */
Dictionary* dict_clone(const Dictionary* const dict)
{
    io_assert(dict != NULL, IO_MSG_NULL_PTR);

    Dictionary* const copy = Dictionary_new(dict->compare, dict->toString);

    /* Lock the data structure to future writers. */
    sync_read_start(dict->rw_sync);

    /* Pre-order iteration should create the same exact Dictionary. */
    dict_Iterator* const iter = dict_iter(dict, PRE_ORDER);
    while (dict_iter_has_next(iter))
    {
        void *value;
        const void* const key = dict_iter_next(iter, &value);
        dict_put(copy, key, value);
    }
    dict_iter_destroy(iter);

    /* Unlock the data structure. */
    sync_read_end(dict->rw_sync);

    return copy;
}

/*
 * Inserts a key/value pair into the Dictionary.
 * If the specified key already exists in the Dictionary, then it's value is replaced with `value`.
 * Θ(log(n))
 */
void dict_put(Dictionary* const dict, const void* const key, const void* const value)
{
    io_assert(dict != NULL, IO_MSG_NULL_PTR);
    io_assert(key != NULL, IO_MSG_NULL_PTR);
    io_assert(value != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(dict->rw_sync);

    int compared;
    dict_Node* const located = dict_binary_search(dict, key, &compared);

    /* Check if we will be inserting a new Node. */
    if (located == NULL || compared != 0)
    {
        dict_Node* const node = dict_Node_new(key, value);

        /* Place the new Node into the dictionary. */
        if (located == NULL)
            dict->root = node;
        else dict_assign_child(located, node, compared > 0);
        dict_enforce_rb_properties(dict, node);
        dict->size++;
    }
    else located->value = value;

    /* Unlock the data structure. */
    sync_write_end(dict->rw_sync);
}

/*
 * Removes all key/value pairs from the Dictionary.
 * Θ(n)
 */
void dict_clear(Dictionary* const dict)
{
    io_assert(dict != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(dict->rw_sync);

    dict_Iterator* const iter = dict_iter(dict, POST_ORDER);
    while (dict_iter_has_next(iter))
    {
        dict_Node* const child = dict_iter_next_node(iter),
                *const parent = PARENT(child);

        if (parent != NULL)
        {
            /* Ensure the iterator cannot access de-allocated memory later on. */
            CHILD(parent, DIRECTION(child, parent)) = NULL;
            iter->current = dict->root;
        }
        dict_Node_destroy(child);
    }
    dict_iter_destroy(iter);

    dict->size = 0;
    dict->root = NULL;

    /* Unlock the data structure. */
    sync_write_end(dict->rw_sync);
}

/*
 * De-constructor function.
 * Θ(n)
 */
void dict_destroy(Dictionary* const dict)
{
    dict_clear(dict);
    sync_destroy(dict->rw_sync);
    mem_free(dict, sizeof(Dictionary));
}

/*
 * Constructor function.
 * Θ(1)
 */
dict_Iterator* dict_iter(const Dictionary* const dict, const enum dict_iter_traversal traverse_type)
{
    io_assert(dict != NULL, IO_MSG_NULL_PTR);
    io_assert(traverse_type >= IN_ORDER && traverse_type <= POST_ORDER, IO_MSG_NOT_SUPPORTED);

    dict_Iterator* const iter = mem_calloc(1, sizeof(dict_Iterator));

    iter->stack = Vector_new(NULL, NULL);
    if (!dict_empty(dict))
    {
        /* Post order needs an additional pointer to work properly. */
        if (traverse_type != PRE_ORDER)
            iter->current = dict->root;
        vect_push_front(iter->stack, dict->root);
    }

    /* Setup function pointers depending on iteration type. */
    switch (traverse_type)
    {
    case IN_ORDER: iter->next = &dict_iter_in_order; break;
    case PRE_ORDER: iter->next = &dict_iter_pre_order; break;
    case POST_ORDER: iter->next = &dict_iter_post_order; break;
    }

    return iter;
}

/*
 * Returns the iterator's current key/value pair and advances it forward.
 * The key will be returned and the value will be assigned to the data of the parameter.
 * Ω(1), O(log(n))
 */
void* dict_iter_next(dict_Iterator* const iter, void **value)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    io_assert(value != NULL, IO_MSG_NULL_PTR);
    io_assert(dict_iter_has_next(iter), IO_MSG_OUT_OF_BOUNDS);

    const dict_Node* const iterated = dict_iter_next_node(iter);
    *value = (void*)iterated->value;

    return (void*)iterated->key;
}

/*
 * Returns true if the iterator has a next key/value pair.
 * Θ(1)
 */
bool dict_iter_has_next(const dict_Iterator* const iter)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    return !vect_empty(iter->stack);
}

/*
 * De-constructor function.
 * Θ(n)
 */
void dict_iter_destroy(dict_Iterator* const iter)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    vect_destroy(iter->stack);
    mem_free(iter, sizeof(dict_Iterator));
}

/*
 * Constructor function.
 * Θ(1)
 */
dict_Node* dict_Node_new(const void* const key, const void* const value)
{
    io_assert(key != NULL, IO_MSG_NULL_PTR);
    io_assert(value != NULL, IO_MSG_NULL_PTR);
    dict_Node* const node = mem_calloc(1, sizeof(dict_Node));
    node->key = key;
    node->value = value;
    return node;
}

/*
 * Returns a Node in the Dictionary whose key matches the specified key.
 * If the Node is found, `compared`'s value will be 0.
 * If the Node is not found, his 'would-be' parent Node is returned. `compared` is
 * set to -1 or 1 depending on if it would have been a left or a right child of that parent.
 * Returns NULL if the Dictionary is empty.
 * Θ(log(n))
 */
dict_Node* dict_binary_search(const Dictionary* const dict, const void* const key, int* const compared)
{
    io_assert(dict != NULL, IO_MSG_NULL_PTR);
    io_assert(key != NULL, IO_MSG_NULL_PTR);
    io_assert(compared != NULL, IO_MSG_NULL_PTR);

    const dict_Node *current = dict->root;

    while (current != NULL)
    {
        /* Compare the current's key with the specified key to determine equality. */
        if (key == current->key) *compared = 0;
        else *compared = dict->compare(key, current->key);

        if (*compared < 0 && current->left != NULL)
            current = current->left;
        else if (*compared > 0 && current->right != NULL)
            current = current->right;
        else return (dict_Node*)current;
    }

    return NULL;
}

/*
 * Returns the Uncle of the specified child Node.
 * Θ(1)
 */
dict_Node* dict_Node_uncle(const dict_Node* const child)
{
    io_assert(child != NULL, IO_MSG_NULL_PTR);
    const dict_Node* const parent = PARENT(child);
    io_assert(parent != NULL, IO_MSG_NULL_PTR);
    const dict_Node* const gparent = PARENT(parent);
    io_assert(gparent != NULL, IO_MSG_NULL_PTR);

    return CHILD(gparent, !DIRECTION(parent, gparent));
}

/*
 * Iterates over the next Node in the Dictionary.
 * The way in which it iterates is defined in the iterator's constructor.
 * Ω(1), O(log(n))
 */
dict_Node* dict_iter_next_node(dict_Iterator* const iter)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    io_assert(dict_iter_has_next(iter), IO_MSG_OUT_OF_BOUNDS);
    return iter->next(iter);
}

/*
 * Iterates the next element using in-order traversal.
 * Iterator's current element must not be NULL.
 * Ω(1), O(log(n))
*/
dict_Node* dict_iter_in_order(dict_Iterator* const iter)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    io_assert(dict_iter_has_next(iter), IO_MSG_OUT_OF_BOUNDS);

    const dict_Node *iterated = NULL;

    while (iterated == NULL)
    {
        /* Peek the top of the Stack. */
        const dict_Node* const next = vect_front(iter->stack);
        vect_pop_front(iter->stack);

        /* Find the in-order successor. */
        // TODO: Create functon or macro for this.
        const dict_Node *successor = next->left;
        while (successor != NULL && successor->right != NULL)
            successor = successor->right;

        /* Only return value when we've gone down the left subtree. */
        if (successor == NULL || successor == iter->current)
            iterated = iter->current = next;
        else
        {
            if (next->right != NULL)
                vect_push_front(iter->stack, next->right);
            vect_push_front(iter->stack, next);
            if (next->left != NULL)
                vect_push_front(iter->stack, next->left);
        }
    }

    return (dict_Node*)iterated;
}

/*
 * Iterates the next element using pre-order traversal.
 * Θ(1)
 */
dict_Node* dict_iter_pre_order(dict_Iterator* const iter)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    io_assert(dict_iter_has_next(iter), IO_MSG_OUT_OF_BOUNDS);

    const dict_Node* const iterated = vect_front(iter->stack);
    vect_pop_front(iter->stack);

    if (iterated->right != NULL)
        vect_push_front(iter->stack, iterated->right);
    if (iterated->left != NULL)
        vect_push_front(iter->stack, iterated->left);

    return (dict_Node*)iterated;
}

/*
 * Iterates the next element using post-order traversal.
 * Iterator's current element must not be NULL.
 * Ω(1), O(log(n))
 */
dict_Node* dict_iter_post_order(dict_Iterator* const iter)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    io_assert(dict_iter_has_next(iter), IO_MSG_OUT_OF_BOUNDS);

    const dict_Node *iterated = NULL;

    /* Keep going until we can actually return a value. */
    while (iterated == NULL)
    {
        /* Peek the top of the Stack. */
        const dict_Node* const next = vect_front(iter->stack);
        /* Only return value when we've gone down both subtrees. */
        if (LEAF(next) || (next->left == iter->current || next->right == iter->current))
        {
            vect_pop_front(iter->stack);
            iterated = iter->current = next;
        }
        else
        {
            if (next->right != NULL)
                vect_push_front(iter->stack, next->right);
            if (next->left != NULL)
                vect_push_front(iter->stack, next->left);
        }
    }

    return (dict_Node*)iterated;
}

/*
 * Returns the height of a given Node..
 * Θ(n)
 */
unsigned int dict_Node_height(const dict_Node* const node)
{
    if (node == NULL) return 0;
    return 1 + max(dict_Node_height(node->left), dict_Node_height(node->right));
}

/*
 * Inspect a Red Node, and adjust the Dictionary in order to preserve red black properties.
 * Ω(1), O(log(n))
 */
void dict_enforce_rb_properties(Dictionary* const dict, dict_Node *child)
{
    io_assert(dict != NULL, IO_MSG_NULL_PTR);
    io_assert(child != NULL, IO_MSG_NULL_PTR);

    bool exit = true;
    /* Case Zero: If child happens to be the root, re-color him BLACK. */
    if (IS_ROOT(child))
        child->color = BLACK;
    else if (COLOR(child) == RED && COLOR(PARENT(child)) == RED)
        exit = false;
    if (exit) return;

    dict_Node* const uncle = dict_Node_uncle(child),
            *parent = PARENT(child),
            *const gparent = PARENT(parent);

    /* Case One: If uncle is RED, re-color and traverse upwards. */
    if (COLOR(uncle) == RED)
    {
        parent->color = BLACK;
        uncle->color = BLACK;
        gparent->color = RED;
        dict_enforce_rb_properties(dict, gparent);
    }
    else
    {
        const bool child_dir = DIRECTION(child, parent),
                parent_dir = DIRECTION(parent, gparent);

        /* Case Two: Angle orientation, prepare for next case. */
        if (child_dir != parent_dir)
        {
            dict_assign_child(gparent, child, !child_dir);
            dict_assign_child(parent, CHILD(child, !child_dir), child_dir);
            dict_assign_child(child, parent, !child_dir);
            SWAP_PTR(child, parent);
        }

        /* Case Three: Line orientation. rotate around the grandparent and re-color. */
        dict_Node* const ggparent = PARENT(gparent);
        if (ggparent != NULL)
            dict_assign_child(ggparent, parent, DIRECTION(gparent, ggparent));
        else
        {
            PARENT(parent) = NULL;
            dict->root = parent;
        }
        dict_assign_child(gparent, CHILD(parent, !parent_dir), parent_dir);
        dict_assign_child(parent, gparent, !parent_dir);
        parent->color = BLACK;
        gparent->color = RED;
    }
}

/*
 * Assigns a child to a parent and a parent to a child.
 * The child is put as the parent's left or right child.
 * Any previous child the parent had in that direction is overwritten.
 * Θ(1)
 */
void dict_assign_child(dict_Node* const parent, dict_Node* const child, const bool direction)
{
    io_assert(parent != NULL || child != NULL, IO_MSG_NULL_PTR);

    /* Parent gains custody of the child. */
    if (parent != NULL)
        CHILD(parent, direction) = child;
    /* Child knows who his parent is. */
    if (child != NULL)
        PARENT(child) = parent;
}

/*
 * Heapifies the Dictionary into a provided array.
 * Helper function for `dict_print_tree`.
 * Θ(n)
 */
void dict_heapify(const dict_Node* const current, const dict_Node** const arr, const unsigned int index)
{
    if (current == NULL) return;
    arr[index] = current;
    dict_heapify(current->left, arr, 2 * index + 1);
    dict_heapify(current->right, arr, 2 * index + 2);
}

/*
 * De-constructor function.
 * Θ(1)
 */
static void dict_Node_destroy(dict_Node* const node)
{
    mem_free(node, sizeof(dict_Node));
}
