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
#define BASE_HASH3 0x12345678 ^ 2166136261UL
#define TABLE_SIZE 400000
#define BLOOM_FILTER_SIZE (5 * TABLE_SIZE)

const unsigned int N = BLOOM_FILTER_SIZE;

// Bloom filter! Can't store pointers as those won't persist well, should extract table0, table1, table2 from store
// to avoid repetitive addition cost to replicate pointers
typedef struct bloom_filter {
    __uint8_t store[N];
    __uint32_t size;
} bloom_filter;

// Hash table, yet to be initialized
bloom_filter* bf;

// Storing bool whether persisted to avoid msync always
bool persisted = false;

inline static void hash_set_all (const char* word) {
    unsigned long hash1 = 5381;
    unsigned long hash2 = 3;
    unsigned long hash3 = BASE_HASH3;
    unsigned long hash4 = 119;
    unsigned long hash5 = 0;

    int c;
    int i = 0;
    while ((c = *word++)) {
        hash1 = ((hash1 << 5) + hash1) + c;
        hash2 = c + (hash2 << 6) + (hash2 << 16) - hash2;
        hash3 ^= c;
        hash3 *= 16777693;
        hash4 = hash4 * 493 + c;
        hash5 += (c << (5 * i++)) + 513;
    }

    bf->store[hash1 % TABLE_SIZE] = 1;
    bf->store[TABLE_SIZE + hash2 % TABLE_SIZE] = 1;
    bf->store[TABLE_SIZE * 2 + hash3 % TABLE_SIZE] = 1;
    bf->store[TABLE_SIZE * 3 + hash4 % TABLE_SIZE] = 1; 
    bf->store[TABLE_SIZE * 4 + hash5 % TABLE_SIZE] = 1; 
    return;
}

inline static bool hash_check_all (const char* word) {
    unsigned long hash1 = 5381;
    unsigned long hash2 = 3;
    unsigned long hash3 = BASE_HASH3;
    unsigned long hash4 = 119;
    unsigned long hash5 = 0;

    int c;
    int i = 0;

    while ((c = *word++)) {
        hash1 = ((hash1 << 5) + hash1) + c;
        hash2 = c + (hash2 << 6) + (hash2 << 16) - hash2;
        hash3 ^= c;
        hash3 *= 16777693;
        hash4 = hash4 * 493 + c;
        hash5 += (c << (5 * i++)) + 513;

    }
    return bf->store[hash1 % TABLE_SIZE] & bf->store[TABLE_SIZE + hash2 % TABLE_SIZE] & bf->store[2 * TABLE_SIZE + hash3 % TABLE_SIZE]
        &  bf->store[3 * TABLE_SIZE + hash4 % TABLE_SIZE] & bf->store[4 * TABLE_SIZE + hash5 % TABLE_SIZE];
}

// Returns true if word is in dictionary, else false
bool check(const char *word)
{
    char lower[LENGTH + 1];
    strcpy(lower, word);
    for (char *c = lower; *c; c++)
        *c = tolower(*c);

    return hash_check_all(lower);
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
    sprintf(path, "%s.datas",  dictionary);

    // If we have a persisted hash table, mmap it open and done
    if (access(path, F_OK) != -1) {
        int fd = open(path, O_RDWR | O_CREAT, (mode_t)0600);

        bf = (bloom_filter*) mmap(0,(sizeof(bloom_filter)), PROT_READ, MAP_SHARED, fd, 0);
        close (fd);
        persisted = true;
        printf("size: %i\n", bf->size);
        return true;
    }

    // Else, create the file, mmap it into existence
    int fd = open(path, O_RDWR | O_CREAT, (mode_t)0600);
    lseek(fd, (sizeof(bloom_filter)-1), SEEK_SET);
    write(fd, "", 1);
    bf = (bloom_filter*) mmap(0,(sizeof(bloom_filter)), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    // Open dictionary
    FILE* f = fopen(dictionary, "r");
    char buffer[LENGTH + 2];

    // Use fgets (significantly faster than fscanf) and remove newline character, hash and store
    while (fgets(buffer, LENGTH + 2, f)) {
        buffer[strlen(buffer) - 1] = 0;
        hash_set_all(buffer); 
        bf->size++;
    }

    // Close for cleanup and store dictsize in persisted hash table
    fclose(f);
    return true;
}

// Returns number of words in dictionary if loaded, else 0 if not yet loaded
unsigned int size(void)
{
    return bf->size;
}

// Unloads dictionary from memory, returning true if successful, else false
bool unload(void)
{
    if (!persisted) {
        msync(bf, sizeof(bloom_filter), MS_SYNC);
    }
    munmap(bf, sizeof(bloom_filter));

    return true;
}
