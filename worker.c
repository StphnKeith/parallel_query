#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#include "freq_list.h"
#include "worker.h"

/* Return an array of FreqRecords for the given index, where each FreqRecord
 * represents the frequency with which the given word occurs in a particular
 * file for the passed in index.
 * The array is in no particular order, except that the last item is a
 * FreqRecord with a frequency of 0 and an empty string filename.
 * Except for the final record, records with a frequency of 0 are excluded
 * from the list.
*/
FreqRecord *get_word(char *word, Node *head, char **file_names) {
	// Find the word in the list
    Node *curr_node = head;
    while (curr_node != NULL && strcmp(curr_node->word, word) != 0) {
        curr_node = curr_node->next;
    }

    FreqRecord *record_array;
    if (curr_node != NULL) { // The word is in the linked list
        int length = 0;
        while (file_names[length] != NULL) {
            length++;
        }

        if ((record_array = malloc(sizeof(FreqRecord) * (length + 1))) == NULL) {
            perror("malloc for long record_array in get_word");
            exit(1);
        }

        // Loop through the word's frequency array and add a FreqRecord to
        // the record_array for each non-zero frequency.
        int j = 0;
        for (int i = 0; i < length; i++) {
            if (curr_node->freq[i] != 0) {
                record_array[j].freq = curr_node->freq[i];
                strncpy(record_array[j].filename, file_names[i], PATHLENGTH);
                j++;
            }
        }

        // Set the last item to be the sentinel
        record_array[j].freq = 0;
        strncpy(record_array[j].filename, "", PATHLENGTH);

    } else { // The word is not in the linked list
        if ((record_array = malloc(sizeof(FreqRecord))) == NULL) {
            perror("malloc for short record_array in get_word");
            exit(1);
        }
        record_array->freq = 0;
        strncpy(record_array->filename, "", PATHLENGTH);
    }
    return record_array;
}

/* Print to standard output the frequency records for a word.
* Use this for your own testing and also for query.c
*/
void print_freq_records(FreqRecord *frp) {
    int i = 0;

    while (frp != NULL && frp[i].freq != 0) {
        printf("%d    %s\n", frp[i].freq, frp[i].filename);
        i++;
    }
}

/* Receives words from the given in file descriptor, gets the frequency for
 * each word by calling get_word on the index in the given directory dirname
 * and writes each record to the out file descriptor.
*/
void run_worker(char *dirname, int in, int out) {
    Node *head = NULL;
    char **filenames = init_filenames();

    char index_path[PATHLENGTH];
    strncpy(index_path, dirname, PATHLENGTH);
    strncat(index_path, "/index", PATHLENGTH - strlen(index_path));

    char filenames_path[PATHLENGTH];
    strncpy(filenames_path, dirname, PATHLENGTH);
    strncat(filenames_path, "/filenames", PATHLENGTH - strlen(index_path));

    read_list(index_path, filenames_path, &head, filenames);

    // For each word read from in
    char word[MAXWORD];
    while (read(in, word, MAXWORD) > 0) {
        // Remove \n at the end of word
        int j = 0;
        while (word[j] != '\0' && word[j] != '\n') {
            j++;
        }
        word[j] = '\0';

        FreqRecord *freq_array = get_word(word, head, filenames);

        // For each non-sentinel FreqRecord
        int i = 0;
        while(strcmp(freq_array[i].filename, "") != 0) {
            if (write(out, &(freq_array[i]), sizeof(FreqRecord)) == -1) {
                perror("write to parent");
            }
            i++;
        }

        // Write sentinel
        if (write(out, &(freq_array[i]), sizeof(FreqRecord)) == -1) {
            perror("write to parent");
        }

        free(freq_array);
    }
}

/* Inserts the given record into the given FRNode linked list frlist.
 * Orders the list from high to low frequencies.
*/
void insert_record(FRNode **frlist, FreqRecord record) {
    FRNode *new_node = malloc(sizeof(FRNode));
    new_node->freq_rec = record;
    new_node->next = NULL;

    // Handle null case
    if (*frlist == NULL) {
        *frlist = new_node;
        return;
    }
    FRNode *head = *frlist;
    int freq = record.freq;

    // Handle front case
    if (head->freq_rec.freq <= freq) {
        *frlist = new_node;
        new_node->next = head;
        return;
    }

    // Loop to node before first frnode with freq <= record's freq
    FRNode *curr_node = head;
    FRNode *next_node = head->next;
    while(next_node != NULL && freq < next_node->freq_rec.freq) {
        curr_node = next_node;
        next_node = next_node->next;
    }

    // Insert
    curr_node->next = new_node;
    new_node->next = next_node;
}

// Create frnode_to_array function that returns an array of the passed in FRNode
// Make sure length is exactly how long the list is
/* Takes a given FRNode linked list and returns a FreqRecord array
 * corresponding to that list.
 * Limits the size of the array to MAXRECORDS plus an extra spot for the
 * sentinel record.
 * Lower frequency records are omitted if there is not enough space.
*/
FreqRecord *frnode_to_array(FRNode *head) {
    FreqRecord *record_array = malloc(sizeof(FreqRecord) * (MAXRECORDS + 1));
    FRNode *curr_node = head;

    // Add each node to the array until either there are no more nodes or
    // the array has run out of space.
    int i = 0;
    while (curr_node != NULL && i < MAXRECORDS) {
        record_array[i] = curr_node->freq_rec;
        curr_node = curr_node->next;
        i++;
    }
    record_array[i].freq = 0;
    strncpy(record_array[i].filename, "", PATHLENGTH);

    return record_array;
}
