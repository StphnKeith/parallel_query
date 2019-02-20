#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "freq_list.h"
#include "worker.h"

/* A command line program that searches for a word within the files in the
 * subdirectories of a given directory, assuming those directories are indexed.
 * One child process is run to search for the word in each subdirectory.
 * No more than 10 child processes will be created, searching a directory with
 * more than 10 subdirectories will result in an exit.
 * The results of the search will be printed to standard output and ordered
 * from higher frequency files to lower frequency files.
*/
int main(int argc, char **argv) {
    char ch;
    char path[PATHLENGTH];
    char *startdir = ".";

    while ((ch = getopt(argc, argv, "d:")) != -1) {
        switch (ch) {
        case 'd':
            startdir = optarg;
            break;
        default:
            fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME]\n");
            exit(1);
        }
    }

    // Open the provided directory or else current working directory
    DIR *dirp;
    if ((dirp = opendir(startdir)) == NULL) {
        perror("opendir");
        exit(1);
    }

    int i = -1;
    int fds[MAXWORKERS][2][2];

    // For each subdirectory
    struct dirent *dp;
    while ((dp = readdir(dirp)) != NULL) {
        // Skip if not proper subdirectory
        if (strcmp(dp->d_name, ".") == 0 ||
            strcmp(dp->d_name, "..") == 0 ||
            strcmp(dp->d_name, ".svn") == 0 ||
            strcmp(dp->d_name, ".git") == 0) {
                continue;
        }

        // Construct the path to the directory
        strncpy(path, startdir, PATHLENGTH);
        strncat(path, "/", PATHLENGTH - strlen(path));
        strncat(path, dp->d_name, PATHLENGTH - strlen(path));
        path[PATHLENGTH - 1] = '\0';

        // Check if the path or the permissions are wrong
        struct stat sbuf;
        if (stat(path, &sbuf) == -1) {
            perror("stat");
            exit(1);
        }

        if (S_ISDIR(sbuf.st_mode)) {
            i++;
            if (i > MAXWORKERS) {
                fprintf(stderr, "too many directories\n");
                exit(1);
            }

            // Make the ith element of fds a pair of in and out pipes
            pipe(fds[i][0]); // in
            pipe(fds[i][1]); // out

            int r = fork();
            if (r == 0) { // Child process
                // Close all read and write ends of pipes before the ith pair
                for (int j = 0; j < i; j++) {
                    close(fds[j][0][1]);
                    close(fds[j][1][0]);
                }
                // Close the write end of the ith in pipe
                close(fds[i][0][1]);
                // Close the read end of the ith out pipe
                close(fds[i][1][0]);

                run_worker(path, fds[i][0][0], fds[i][1][1]);

                // Close the write end of out
                close(fds[i][1][1]);

                // Exit to avoid forking child processes on next loop
                exit(0);

            } else if (r > 0) { // Parent process
                // Close the read end of the ith in pipe
                close(fds[i][0][0]);
                // Close the write end of the ith out pipe
                close(fds[i][1][1]);
            } else {
                perror("fork");
                exit(1);
            }
        }
    }

    // Only the parent makes it here
    // Increment i so that it represents the index after the last pipe pair
    i++;
    FRNode *fr_head = NULL;

    char word[MAXWORD];
    FreqRecord *freq_array;

    // For each word
    while (fgets(word, MAXWORD, stdin) != NULL) {
        // Pass the word to each child process
        for (int j = 0; j < i; j++) {
            if (write(fds[j][0][1], word, MAXWORD) == -1) {
                perror("write to child");
            }
        }
        
        // For each child
        for (int j = 0; j < i; j++) {
            FreqRecord record_from_out;
            if ((read(fds[j][1][0], &record_from_out, sizeof(FreqRecord))) < 0)
            {
                perror("read from child");
                exit(1);
            }

            // For each FreqRecord read from jth child
            while(record_from_out.freq != 0) {
                insert_record(&fr_head, record_from_out);
                if ((read(fds[j][1][0], &record_from_out, sizeof(FreqRecord))) < 0) {
                    perror("read from child");
                    exit(1);
                }
            }
        }

        freq_array = frnode_to_array(fr_head);
        print_freq_records(freq_array);
        free(freq_array);

        while (fr_head != NULL) {
            FRNode *next_node = fr_head->next;
            free(fr_head);
            fr_head = next_node;
        }
    }

    // Close the inputs to each child, causing each one to exit
    for (int j = 0; j < i; j++) {
        close(fds[j][0][1]);
    }

    if (closedir(dirp) < 0) {
        perror("closedir");
    }

	return 0;
}
