#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#include "freq_list.h"
#include "worker.h"

/* This program tests get_word by building a test index with three files.
 * f1 contains the word apple.
 * f2 contains the words apple twice, and orange four times.
 * f3 contains the words apple three times, orange five times,
 * and banana six times.
 * The files are listed in order f1, f2, f3 in filenames.
*/
int main() {
    // Build a test index
    Node *head = malloc(sizeof(Node));
    strncpy(head->word, "apple", MAXWORD);
    head->freq[0] = 1;
    head->freq[1] = 2;
    head->freq[2] = 3;

    Node *second = malloc(sizeof(Node));
    head->next = second;
    strncpy(second->word, "orange", MAXWORD);
    second->freq[0] = 0;
    second->freq[1] = 4;
    second->freq[2] = 5;

    Node *third = malloc(sizeof(Node));
    second->next = third;
    strncpy(third->word, "banana", MAXWORD);
    third->freq[0] = 0;
    third->freq[1] = 0;
    third->freq[2] = 6;
    third->next = NULL;

    char **filenames = init_filenames();
    filenames[0] = malloc(sizeof(char) * MAXWORD);
    strncpy(filenames[0], "f1", MAXWORD);

    filenames[1] = malloc(sizeof(char) * MAXWORD);
    strncpy(filenames[1], "f2", MAXWORD);

    filenames[2] = malloc(sizeof(char) * MAXWORD);
    strncpy(filenames[2], "f3", MAXWORD);


    // Begin testing
    int errorval = 0;

    // First word
    // get_word("apple") == [ ("f1",1), ("f2",2), ("f3",3), ("",0) ]
    FreqRecord *freq_array = get_word("apple", head, filenames);
    // Frequencies
    if (freq_array[0].freq != 1) {
        printf("apple frequency f1 fail\n");
        errorval = 1;
    }
    if (freq_array[1].freq != 2) {
        printf("apple frequency f2 fail\n");
        errorval = 1;
    }
    if (freq_array[2].freq != 3) {
        printf("apple frequency f3 fail\n");
        errorval = 1;
    }
    if (freq_array[3].freq != 0) {
        printf("apple frequency sentinel fail\n");
        errorval = 1;
    }
    // Filenames
    if (strcmp(freq_array[0].filename, "f1") != 0) {
        printf("apple filename f1 fail\n");
        errorval = 1;
    }
    if (strcmp(freq_array[1].filename, "f2") != 0) {
        printf("apple filename f2 fail\n");
        errorval = 1;
    }
    if (strcmp(freq_array[2].filename, "f3") != 0) {
        printf("apple filename f3 fail\n");
        errorval = 1;
    }
    if (strcmp(freq_array[3].filename, "") != 0) {
        printf("apple filename sentinel fail\n");
        errorval = 1;
    }
    free(freq_array);

    // Second word
    // get_word("orange") == [ ("f2",4), ("f3",5), ("",0) ]
    freq_array = get_word("orange", head, filenames);
    // Frequencies
    if (freq_array[0].freq != 4) {
        printf("orange frequency f2 fail\n");
        errorval = 1;
    }
    if (freq_array[1].freq != 5) {
        printf("orange frequency f3 fail\n");
        errorval = 1;
    }
    if (freq_array[2].freq != 0) {
        printf("orange frequency sentinel fail\n");
        errorval = 1;
    }
    // Filenames
    if (strcmp(freq_array[0].filename, "f2") != 0) {
        printf("orange filename f2 fail\n");
        errorval = 1;
    }
    if (strcmp(freq_array[1].filename, "f3") != 0) {
        printf("orange filename f3 fail\n");
        errorval = 1;
    }
    if (strcmp(freq_array[2].filename, "") != 0) {
        printf("orange filename sentinel fail\n");
        errorval = 1;
    }
    free(freq_array);

    // Third word
    // get_word("banana") == [ ("f3",6), ("",0) ]
    freq_array = get_word("banana", head, filenames);
    //Frequencies
    if (freq_array[0].freq != 6) {
        printf("banana frequency f3 fail\n");
        errorval = 1;
    }
    if (freq_array[1].freq != 0) {
        printf("banana frequency sentinel fail\n");
        errorval = 1;
    }
    // Filenames
    if (strcmp(freq_array[0].filename, "f3") != 0) {
        printf("banana filename f3 fail\n");
        errorval = 1;
    }
    if (strcmp(freq_array[1].filename, "") != 0) {
        printf("banana filename sentinel fail\n");
        errorval = 1;
    }
    free(freq_array);

    // Too short word
    // strncpy(word, "few", MAXWORD);
    freq_array = get_word("few", head, filenames);
    if (freq_array[0].freq != 0) {
        printf("few frequency sentinel fail\n");
        errorval = 1;
    }
    if (strcmp(freq_array[0].filename, "") != 0) {
        printf("few filename sentinel fail\n");
        errorval = 1;
    }
    free(freq_array);

    // Non-existent word
    // strncpy(word, "gone", MAXWORD);
    freq_array = get_word("gone", head, filenames);
    if (freq_array[0].freq != 0) {
        printf("gone frequency sentinel fail\n");
        errorval = 1;
    }
    if (strcmp(freq_array[0].filename, "") != 0) {
        printf("gone filename sentinel fail\n");
        errorval = 1;
    }
    free(freq_array);

    // Mis-matching case word
    // strncpy(word, "Banana", MAXWORD);
    freq_array = get_word("Banana", head, filenames);
    if (freq_array[0].freq != 0) {
        printf("Banana frequency sentinel fail\n");
        errorval = 1;
    }
    if (strcmp(freq_array[0].filename, "") != 0) {
        printf("Banana filename sentinel fail\n");
        errorval = 1;
    }
    free(freq_array);

    // Whitespace word
    // strncpy(word, "banana\n", MAXWORD);
    freq_array = get_word("banana\n", head, filenames);
    if (freq_array[0].freq != 0) {
        printf("banana\\n frequency sentinel fail\n");
        errorval = 1;
    }
    if (strcmp(freq_array[0].filename, "") != 0) {
        printf("banana\\n filename sentinel fail\n");
        errorval = 1;
    }
    free(freq_array);
    
    if (errorval == 0) {
        printf("all tests passed\n");
    }

    free(third);
    free(second);
    free(head);
    free(filenames[0]);
    free(filenames[1]);
    free(filenames[2]);
    free(filenames);
    
    return errorval;
}
