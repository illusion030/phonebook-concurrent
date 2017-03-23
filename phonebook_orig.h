#ifndef _PHONEBOOK_H
#define _PHONEBOOK_H

#define MAX_LAST_NAME_SIZE 16

/* original version */

typedef struct __PHONE_BOOK_ENTRY {
    char lastName[MAX_LAST_NAME_SIZE];
    char firstName[16];
    char email[16];
    char phone[10];
    char cell[10];
    char addr1[16];
    char addr2[16];
    char city[16];
    char state[2];
    char zip[5];
    struct __PHONE_BOOK_ENTRY *pNext;
} entry;

extern struct __PHONEBOOK_API__ {
    entry *(*findLastName)(char lastname[], entry *pHead);
    entry *(*append)(char *fileName);
    void (*write)(double cpu_time[]);
    void (*free)(entry *pHead);
} pb;

entry *append(char lastName[], entry *e);
entry *orig_findLastName(char lastname[], entry *pHead);
entry *orig_append(char *fileName);
void orig_write(double cpu_time[]);
void orig_free(entry *pHead);

#endif
