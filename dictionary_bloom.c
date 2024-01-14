// Implements a dictionary's functionality

#include <ctype.h>
#include <stdbool.h>

#include "dictionary.h"
#include "strings.h"
#include "stdio.h"
#include "string.h"

#define NODE_CAPACITY 5
#define base1 300043
#define base2 2 * base1 
// #define N 1200000


// TODO: Choose number of buckets in hash table

__uint32_t dictsize = 0;


const unsigned int N = 3 * base1;

// Hash table
__uint8_t table[N];

inline static void hash_set_all (const char* word) {
    unsigned long hash3 = 0x12345678;

    // See: https://github.com/aappleby/smhasher/blob/master/src/Hashes.cpp
    hash3 ^= 2166136261UL;

    int c;

    while ((c = *word++)) {
        hash3 ^= c;
        hash3 *= 16777619;
    }

    table[hash3 % N] = 1;
    return;
}

inline static bool hash_check_all (const char* word) {
    unsigned long hash1 = 5381;
    unsigned long hash2 = 0;
    unsigned long hash3 = 0x12345678;

    // See: https://github.com/aappleby/smhasher/blob/master/src/Hashes.cpp
    hash3 ^= 2166136261UL;

    int c;

    while ((c = *word++)) {
        hash1 = ((hash1 << 5) + hash1) + c;
        hash2 = c + (hash2 << 6) + (hash2 << 16) - hash2;
        hash3 ^= c;
        hash3 *= 16777619;
    }
    return table[hash1 % base1] & table[base1 + (hash2 % base1)] & table[base2 + (hash3 % base1)];
}

// Returns true if word is in dictionary, else false
bool check(const char *word)
{
    char lower[LENGTH];
    strcpy(lower, word);
    for (char *c = lower; *c; c++)
        *c = tolower(*c);
    return hash_check_all(lower);
}

// // Hashes word to a number - looking at speller.c this is never called directly
// So let's actually not use it, try to use inlining for performance gains.
unsigned int hash(const char *word)
{
    // TODO: Find the ultimate question
    return 42;
}

// Loads dictionary into memory, returning true if successful, else false
bool load(const char *dictionary)
{
    FILE* f = fopen(dictionary, "r");
    char buffer[LENGTH + 2];
    while (fgets(buffer, LENGTH + 2, f)) {
        buffer[strlen(buffer) - 1] = 0;
        hash_set_all(buffer);
        dictsize++;
    }
    fclose(f);
    return true;
}

// Returns number of words in dictionary if loaded, else 0 if not yet loaded
unsigned int size(void)
{
    return dictsize;
}

// Unloads dictionary from memory, returning true if successful, else false
bool unload(void)
{
    return true;
}
