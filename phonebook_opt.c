#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <assert.h>

#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>

#include "phonebook_opt.h"
#include "debug.h"
#include "text_align.h"

#define ALIGN_FILE "align.txt"

#ifndef THREAD_NUM
#define THREAD_NUM 4
#endif

char *map;
entry *entry_pool;
thread_arg *thread_args[THREAD_NUM];
off_t file_size;
int fd;

entry *opt_findLastName(char lastname[], entry *pHead)
{
    size_t len = strlen(lastname);
    while (pHead) {
        if (strncasecmp(lastname, pHead->lastName, len) == 0
                && (pHead->lastName[len] == '\n' ||
                    pHead->lastName[len] == '\0')) {
            pHead->lastName[len] = '\0';
            if (!pHead->dtl)
                pHead->dtl = (pdetail) malloc(sizeof(detail));
            return pHead;
        }
        DEBUG_LOG("find string = %s\n", pHead->lastName);
        pHead = pHead->pNext;
    }
    return NULL;
}

thread_arg *createThread_arg(char *data_begin, char *data_end,
                             int threadID, int numOfThread,
                             entry *entryPool)
{
    thread_arg *new_arg = (thread_arg *) malloc(sizeof(thread_arg));

    new_arg->data_begin = data_begin;
    new_arg->data_end = data_end;
    new_arg->threadID = threadID;
    new_arg->numOfThread = numOfThread;
    new_arg->lEntryPool_begin = entryPool;
    new_arg->lEntry_head = new_arg->lEntry_tail = entryPool;
    return new_arg;
}

/**
 * Generate a local linked list in thread.
 */
void threads_append(void *arg)
{
    struct timespec start, end;
    double cpu_time;

    clock_gettime(CLOCK_REALTIME, &start);

    thread_arg *t_arg = (thread_arg *) arg;
    int count = 0;
    entry *j = t_arg->lEntryPool_begin;
    for (char *i = t_arg->data_begin; i < t_arg->data_end;
            i += MAX_LAST_NAME_SIZE * t_arg->numOfThread,
            j += t_arg->numOfThread, count++) {
        /* Append the new at the end of the local linked list */
        t_arg->lEntry_tail->pNext = j;
        t_arg->lEntry_tail = t_arg->lEntry_tail->pNext;
        t_arg->lEntry_tail->lastName = i;
        t_arg->lEntry_tail->pNext = NULL;
        t_arg->lEntry_tail->dtl = NULL;
        DEBUG_LOG("thread %d t_argend string = %s\n",
                  t_arg->threadID, t_arg->lEntry_tail->lastName);
    }
    clock_gettime(CLOCK_REALTIME, &end);
    cpu_time = diff_in_second(start, end);

    DEBUG_LOG("thread %d take %lf sec, count %d\n", t_arg->threadID, cpu_time, count);

    pthread_exit(NULL);
}

void show_entry(entry *pHead)
{
    while (pHead) {
        printf("%s", pHead->lastName);
        pHead = pHead->pNext;
    }
}

static double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec-t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}

entry *opt_append(char *fileName)
{
    int i = 0;

    /* File preprocessint */
    text_align(fileName, ALIGN_FILE, MAX_LAST_NAME_SIZE);
    fd = open(ALIGN_FILE, O_RDONLY | O_NONBLOCK);
    file_size = fsize(ALIGN_FILE);

    /* Build the entry */
    entry *pHead, *e;
    printf("size of entry : %lu bytes\n", sizeof(entry));

    pthread_t threads[THREAD_NUM];

    /* Allocate the resorce at first */
    map = mmap(NULL, file_size,
               PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    assert(map && "mmap error");
    entry_pool = (entry *)malloc(sizeof(entry) *
                                 file_size / MAX_LAST_NAME_SIZE);
    assert(entry_pool && "entry_pool error");

    /* Prepare for multi-threading */
    //pthread_setconcurrency(THREAD_NUM + 1);
    for (i = 0; i < THREAD_NUM; i++)
        // Created by malloc, remenber to free them.
        thread_args[i] = createThread_arg(map + MAX_LAST_NAME_SIZE * i, map + file_size, i, THREAD_NUM, entry_pool + i);

    /* Deliver the jobs to all threads and wait for completing */
    for (i = 0; i < THREAD_NUM; i++)
        pthread_create(&threads[i], NULL, (void *)&threads_append,
                       (void *)thread_args[i]);

    for (i = 0; i < THREAD_NUM; i++)
        pthread_join(threads[i], NULL);

    /* Connect the linked list of each thread */
    pHead = thread_args[0]->lEntry_head->pNext;
    DEBUG_LOG("Connect %d head string %s %p\n", 0,
              pHead->lastName, thread_args[0]->data_begin);
    e = thread_args[0]->lEntry_tail;
    DEBUG_LOG("Connect %d tail string%s %p\n", 0,
              e->lastName, thread_args[0]->data_begin);
    DEBUG_LOG("round %d\n", 0);
    for (i = 1; i < THREAD_NUM; i++) {
        e->pNext = thread_args[i]->lEntry_head->pNext;
        DEBUG_LOG("Connect %d head string %s %p\n", i,
                  e->pNext->lastName, thread_args[i]->data_begin);

        e = thread_args[i]->lEntry_tail;
        DEBUG_LOG("Connect %d tail string%s %p\n", i,
                  e->lastName, thread_args[i]->data_begin);
        DEBUG_LOG("round %d\n", i);
    }

    return pHead;
}

void opt_write(double cpu_time[])
{
    FILE *output;
    output = fopen("opt.txt", "a");
    fprintf(output, "pb_append() pb_findLastName() %lf %lf\n",
            cpu_time[0], cpu_time[1]);
    fclose(output);
}

void opt_free(entry *pHead)
{
    entry *e = pHead;
    /* free the location detail */
    while (e) {
        free(e->dtl);
        e = e->pNext;
    }
    free(entry_pool);
    for (int i = 0; i < THREAD_NUM; i++)
        free(thread_args[i]);
    munmap(map, file_size);
    close(fd);
}

struct __PHONEBOOK_API__ pb = {
    .findLastName = opt_findLastName,
    .append = opt_append,
    .write = opt_write,
    .free = opt_free,
};
