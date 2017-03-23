#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "phonebook_orig.h"

/* original version */
entry *orig_findName(char lastname[], entry *pHead)
{
    while (pHead) {
        if (strcasecmp(lastname, pHead->lastName) == 0)
            return pHead;
        pHead = pHead->pNext;
    }
    return NULL;
}

entry *append(char lastName[], entry *e)
{
    /* allocate memory for the new entry and put lastName */
    e->pNext = (entry *) malloc(sizeof(entry));
    e = e->pNext;
    strcpy(e->lastName, lastName);
    e->pNext = NULL;

    return e;
}

entry *orig_append (char *fileName)
{
    FILE *fp;
    int i = 0;
    char line[MAX_LAST_NAME_SIZE];

    /* File preprocessing */
    /* check file opening */
    fp = fopen(fileName, "r");
    if (!fp) {
        printf("cannot open the file\n");
        return NULL;
    }

    /*Build the entry */
    entry *pHead, *e;
    printf("size of entry : %lu bytes\n", sizeof(entry));

    pHead = (entry *) malloc(sizeof(entry));
    e = pHead;
    e->pNext = NULL;

    while (fgets(line, sizeof(line), fp)) {
        while (line[i] != '\0')
            i++;
        line[i - 1] = '\0';
        i = 0;
        e = append(line, e);
    }

    /* close file as soon as possible */
    fclose(fp);

    return pHead;
}


void orig_write(double cpu_time[])
{
    FILE *output;
    output = fopen("orig.txt", "a");
    fprintf(output, "phonebook_append() phonebook_findName() %lf %lf\n", cpu_time[0], cpu_time[1]);
    fclose(output);
}

void orig_free(entry *pHead)
{
    entry *e;
    while (pHead) {
        e = pHead;
        pHead = pHead->pNext;
        free(e);
    }
}

struct __PHONEBOOK_API__ Phonebook = {
    .phonebook_findName = orig_findName,
    .phonebook_append = orig_append,
    .phonebook_write = orig_write,
    .phonebook_free = orig_free,
};
