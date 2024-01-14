// Implements a dictionary's functionality

#include <ctype.h>
#include <stdbool.h>

#include "dictionary.h"
#include "strings.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#define NODE_CAPACITY 5
#define BASE_HASH 0x12345678 ^ 2166136261UL

// Represents a node in a hash table
typedef struct node
{
    __uint8_t stored;
    char words[NODE_CAPACITY][LENGTH + 1];
} node;

// Picking roughly double the number of words, could probably do some tuning here
const unsigned int N = 400000;

// Hash table struct stores the size and the actual table.
// Needs to store the size so that it can print the size at the end
// Even when loading from a persisted hash table.
typedef struct hash_table {
    __uint32_t size;
    node table[N];
} hash_table;

// Dictionary size variable. Should probably erase this and just use the hash table but could
// test whether it's faster to transfer it at the end
__uint32_t dictsize = 0;

// Hash table, yet to be initialized
hash_table* ht;

bool persisted = false;

// See: https://github.com/aappleby/smhasher/blob/master/src/Hashes.cpp
inline static __uint32_t my_hash (const char* word) {
    unsigned long hash = BASE_HASH;
    int c;

    while ((c = *word++)) {
        hash ^= c;
        hash *= 16777619;
    }
    return hash % N;
}

// Returns true if word is in dictionary, else false
bool check(const char *word)
{
    char lower[LENGTH + 1];
    strcpy(lower, word);
    for (char *c = lower; *c; c++)
        *c = tolower(*c);

    __uint32_t hash = my_hash(lower);


    node* pos = &ht->table[hash];
    for (__uint8_t i = 0; i < pos->stored; i++) {
        if (!strcmp(pos->words[i], lower)) {
            return true;
        }
    }
    return false;
}

// Hashes word to a number - looking at speller.c this is never called directly
// So let's actually not use it, try to use inlining for performance gains.
unsigned int hash(const char *word)
{
    // TODO: Find the ultimate question
    return 42;
}

// Loads dictionary into memory, returning true if successful, else false
bool load(const char *dictionary)
{
    // Setup path to the persisted hash table
    char path[LENGTH + 10];
    sprintf(path, "%s.data",  dictionary);

    // If we have a persisted hash table, mmap it open and done
    if (access(path, F_OK) != -1) {
        int fd = open(path, O_RDWR | O_CREAT, (mode_t)0600);

        ht = (hash_table*) mmap(0,(sizeof(hash_table)), PROT_READ, MAP_PRIVATE, fd, 0);
        close (fd);
        persisted = true;
        return true;
    }

    // Else, create the file, mmap it into existence
    int fd = open(path, O_RDWR | O_CREAT, (mode_t)0600);
    lseek(fd, (sizeof(hash_table)-1), SEEK_SET);
    write(fd, "", 1);
    ht = (hash_table*) mmap(0,(sizeof(hash_table)), PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
    close(fd);

    // Open dictionary
    FILE* f = fopen(dictionary, "r");
    char buffer[LENGTH + 2];

    // Use fgets (significantly faster than fscanf) and remove newline character, hash and store
    while (fgets(buffer, LENGTH + 2, f)) {
        buffer[strlen(buffer) - 1] = 0;
        __uint32_t pos = my_hash(buffer); 
        strcpy(ht->table[pos].words[ht->table[pos].stored++], buffer);
        dictsize++;
    }

    // Close for cleanup and store dictsize in persisted hash table
    fclose(f);
    ht->size = dictsize;
    return true;
}

// Returns number of words in dictionary if loaded, else 0 if not yet loaded
unsigned int size(void)
{
    return ht->size;
}

// Unloads dictionary from memory, returning true if successful, else false
bool unload(void)
{

    msync(ht, sizeof(hash_table), MS_SYNC);
    if (!persisted) {
        munmap(ht, sizeof(hash_table));
    }
    return true;
}
