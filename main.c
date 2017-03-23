#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>

#include IMPL

#define DICT_FILE "./dictionary/words.txt"

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

int main(int argc, char *argv[])
{
    struct timespec start, end;
    double cpu_time[2];
    entry *e, *pHead;

    /* Start timing */
    clock_gettime(CLOCK_REALTIME, &start);
    pHead = Phonebook.phonebook_append(DICT_FILE);
    /* Stop timing */
    clock_gettime(CLOCK_REALTIME, &end);

    cpu_time[0] = diff_in_second(start, end);

    e = pHead;

    assert(Phonebook.phonebook_findName("zyxel", e) &&
           "Did you implement findName() in " IMPL "?");
    assert(0 == strcmp(Phonebook.phonebook_findName("zyxel", e)->lastName, "zyxel"));

#if defined(__GNUC__)
    __builtin___clear_cache((char *) pHead, (char *) pHead + sizeof(entry));
#endif
    /* Compute the execution time */
    clock_gettime(CLOCK_REALTIME, &start);
    Phonebook.phonebook_findName("zyxel", e);
    clock_gettime(CLOCK_REALTIME, &end);
    cpu_time[1] = diff_in_second(start, end);

    /* Write the execution time to file. */
    Phonebook.phonebook_write(cpu_time);

    printf("execution time of phonebook_append() : %lf sec\n", cpu_time[0]);
    printf("execution time of phonebook_findName() : %lf sec\n", cpu_time[1]);

    /* Release memory */
    Phonebook.phonebook_free(pHead);

    return 0;
}
