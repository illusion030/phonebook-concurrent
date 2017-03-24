#ifndef _PHONEBOOK_H
#define _PHONEBOOK_H

#include <pthread.h>
#include <time.h>

#define MAX_LAST_NAME_SIZE 16

#define OPT 1

typedef struct _detail {
    char firstName[16];
    char email[16];
    char phone[10];
    char cell[10];
    char addr1[16];
    char addr2[16];
    char city[16];
    char state[2];
    char zip[5];
} detail;

typedef detail *pdetail;

typedef struct __PHONE_BOOK_ENTRY {
    char *lastName;
    struct __PHONE_BOOK_ENTRY *pNext;
    pdetail dtl;
} entry;

typedef struct _thread_argument {
    char *data_begin;
    char *data_end;
    int threadID;
    int numOfThread;
    entry *lEntryPool_begin;    /* The local entry pool */
    entry *lEntry_head;	/* local entry linked list */
    entry *lEntry_tail;	/* local entry linked list */
} thread_arg;

extern struct __PHONEBOOK_API__ {
    entry *(*findLastName)(char lastname[], entry *pHead);
    entry *(*append)(char *fileName);
    void (*write)(double cpu_time[]);
    void (*free)(entry *pHead);
} phonebook_opt;

thread_arg *createThread_arg(char *data_begin, char *data_end,
                             int threadID, int numOfThread,
                             entry *entryPool);

entry *opt_findLastName(char lastname[], entry *pHead);
void threads_append(void *arg);
void show_entry(entry *pHead);
static double diff_in_second(struct timespec t1, struct timespec t2);
entry *opt_append(char *fileName);
void opt_write(double cpu_time[]);
void opt_free(entry *pHead);

#endif
