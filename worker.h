#ifndef WORKER_H
#define WORKER_H

#define PATHLENGTH 128
#define MAXRECORDS 100

#define MAXWORKERS 10

// This data structure is used by the workers to prepare the output
// to be sent to the master process.

typedef struct {
    int freq;
    char filename[PATHLENGTH];
} FreqRecord;

struct frnode {
    FreqRecord freq_rec;
    struct frnode *next;
};
typedef struct frnode FRNode;

FreqRecord *get_word(char *word, Node *head, char **file_names);
void print_freq_records(FreqRecord *frp);
void run_worker(char *dirname, int in, int out);
void insert_record(FRNode **frlist, FreqRecord record);
FreqRecord *frnode_to_array(FRNode *head);

#endif /* WORKER_H */
