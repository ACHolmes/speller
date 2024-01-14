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
#include <sys/stat.h>

#define NODE_CAPACITY 5
#define BASE_HASH3 0x12345678 ^ 2166136261UL
#define TABLE_SIZE 2100013
#define BLOOM_FILTER_SIZE (2 * TABLE_SIZE)

const unsigned int N = BLOOM_FILTER_SIZE;

// Bloom filter! Can't store pointers as those won't persist well, should extract table0, table1, table2 from store
// to avoid repetitive addition cost to replicate pointers
// typedef struct bloom_filter {
//     __uint8_t store[N];
//     __uint32_t size;
// } bloom_filter;

// // Hash table, yet to be initialized
// bloom_filter* bf;

// // Storing bool whether persisted to avoid msync always
// bool persisted = false;

__uint8_t table1[TABLE_SIZE];
__uint8_t table2[TABLE_SIZE];

__uint32_t dictsize;
char dict[1439045];


inline static bool hash_check_all (const char* word) {
    unsigned long hash1 = 5387;
    unsigned long hash2 = 2;
    int c;

    while ((c = *word++)) {
        // Equally could use ^, or use 'a' or 'A' more directly
        c = c | 0x20;
        hash1 = ((hash1 << 5) + (hash1 << 2)) + c;
        hash2 = c + (hash2 << 6) + (hash2 << 16) - hash2;

    }
    return table1[hash1 % TABLE_SIZE] & table2[hash2 % TABLE_SIZE];
}

// Returns true if word is in dictionary, else false
bool check(const char *word)
{
    return hash_check_all(word);
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
    __uint32_t local_dictsize = 1;
    struct stat sb;

    int fd = open(dictionary, O_RDONLY);
    fstat(fd, &sb);
    read(fd, &dict, sb.st_size);
    unsigned long hash1 = 5387;
    unsigned long hash2 = 2;
    for (int i = 0; i < sb.st_size; i++) {
        if (dict[i] != '\n') {
            hash1 = ((hash1 << 5) + (hash1 << 2)) + dict[i];
            hash2 = dict[i] + (hash2 << 6) + (hash2 << 16) - hash2;
        } else {
            table1[hash1 % TABLE_SIZE] = 1;
            table2[hash2 % TABLE_SIZE] = 1;
            hash1 = 5387;
            hash2 = 2;
            local_dictsize++;
        }
    }

    dictsize = local_dictsize;
    close(fd);
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
