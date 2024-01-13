// Implements a dictionary's functionality

#include <ctype.h>
#include <stdbool.h>

#include "dictionary.h"
#include "strings.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"
 #include <time.h>

#define NODE_CAPACITY 5

// Represents a node in a hash table
typedef struct node
{
    __uint8_t stored;
    char words[NODE_CAPACITY][LENGTH + 1];
} node;

// TODO: Choose number of buckets in hash table
const unsigned int N = 400000;

__uint32_t dictsize = 0;

// Hash table
int table[N];

// My hash using djb2
inline static __uint32_t my_hash (const char* word) {
    unsigned long hash = 5381;
    int c;

    while ((c = tolower(*word++)))
        hash = ((hash << 5) + hash) + c;

    return hash % N;
}

// Returns true if word is in dictionary, else false
bool check(const char *word)
{
    __uint32_t hash =  my_hash(word);

    char lower[LENGTH];
    strcpy(lower, word);
    for (char *c = lower; *c; c++)
        *c = tolower(*c);

    node* pos = &table[hash];
    for (__uint8_t i = 0; i < pos->stored; i++) {
        if (!strcmp(pos->words[i], lower)) {
            return true;
        }
    }
    return false;
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
    clock_t start, end;
    start = clock();
    FILE* f = fopen(dictionary, "r");
    char buffer[LENGTH + 2];
    while (fgets(buffer, LENGTH + 2, f)) {
        // __uint8_t i = 0;
        // for (; buffer[i] != '\n'; i++) {}
        // buffer[i] = 0;
        buffer[strlen(buffer) - 1] = 0;
        // strtok(buffer, "\n");
        __uint32_t pos = my_hash(buffer); 
        strcpy(table[pos].words[table[pos].stored++], buffer);
        dictsize++;
    }
    fclose(f);
    end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    // printf("%d\n", cpu_time_used);
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
