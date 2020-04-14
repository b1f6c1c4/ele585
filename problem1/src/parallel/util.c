#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "util.h"

void check(int ret)
{
    if (ret)
        exit(1);
}

sync_clock_t *make_sync_clock(int nth)
{
    sync_clock_t *sc = malloc(sizeof(sync_clock_t));
    sc->total = nth;
    sc->started = 0;
    sc->running = 0;
    check(pthread_cond_init(&sc->before_cv, NULL));
    check(pthread_mutex_init(&sc->before_mp, NULL));
    check(pthread_cond_init(&sc->after_cv, NULL));
    check(pthread_mutex_init(&sc->after_mp, NULL));
    sc->tinfos = malloc(sizeof(tinfo_t) * nth);
    return sc;
}

void free_sync_clock(sync_clock_t *sc)
{
    free(sc->tinfos);
    check(pthread_cond_destroy(&sc->before_cv));
    check(pthread_mutex_destroy(&sc->before_mp));
    check(pthread_cond_destroy(&sc->after_cv));
    check(pthread_mutex_destroy(&sc->after_mp));
    free(sc);
}

void before_execution(sync_clock_t *sc)
{
    // Notify main thread that I'm ready
    check(pthread_mutex_lock(&sc->after_mp));
    sc->running++;
    check(pthread_cond_signal(&sc->after_cv));
    check(pthread_mutex_unlock(&sc->after_mp));

    // Wait for signal from main thread
    check(pthread_mutex_lock(&sc->before_mp));
    while (!sc->started)
        check(pthread_cond_wait(&sc->before_cv, &sc->before_mp));
    check(pthread_mutex_unlock(&sc->before_mp));
}

void after_execution(sync_clock_t *sc)
{
    // Notify main thread that I'm finished
    check(pthread_mutex_lock(&sc->after_mp));
    sc->running--;
    check(pthread_cond_signal(&sc->after_cv));
    check(pthread_mutex_unlock(&sc->after_mp));
}

void *thread_start(void *args)
{
    tinfo_t *tinfo = (tinfo_t *)(args);

    before_execution(tinfo->sc);
    tinfo->entry(tinfo->nth, tinfo->arg);
    after_execution(tinfo->sc);

    return NULL;
}

void run_all(sync_clock_t *sc, void (*entry)(int, void *), void *arg)
{
    int i;
    for (i = 0; i < sc->total; i++)
    {
        tinfo_t *tinfo = &sc->tinfos[i];
        tinfo->sc = sc;
        tinfo->entry = entry;
        tinfo->nth = i;
        tinfo->arg = arg;

        check(pthread_create(&sc->tinfos[i].pth, NULL, &thread_start, tinfo));
    }

    // Wait for all threads to be ready
    check(pthread_mutex_lock(&sc->after_mp));
    while (sc->running != sc->total)
        check(pthread_cond_wait(&sc->after_cv, &sc->after_mp));
    check(pthread_mutex_unlock(&sc->after_mp));

    // Signal all threads
    check(pthread_mutex_lock(&sc->before_mp));
    sc->started = 1;
    check(pthread_cond_broadcast(&sc->before_cv));
    check(pthread_mutex_unlock(&sc->before_mp));

    // Get the starting time
    clock_gettime(CLOCK_REALTIME, &sc->t1);

    // Wait for all threads to be finished
    check(pthread_mutex_lock(&sc->after_mp));
    while (!sc->started || sc->running != 0)
        check(pthread_cond_wait(&sc->after_cv, &sc->after_mp));
    check(pthread_mutex_unlock(&sc->after_mp));
}

void finalize(sync_clock_t *sc)
{
    // Get the ending time
    clock_gettime(CLOCK_REALTIME, &sc->t2);

    // Join all threads
    int i;
    for (i = 0; i < sc->total; i++)
        check(pthread_join(sc->tinfos[i].pth, NULL));
}

void load_data(char * file_name, long long input_data_length, unsigned char ** parsed_array, int number_buckets)
{
    FILE * fr;
    unsigned char * local_parsed_array = malloc(sizeof(unsigned char)*input_data_length);
    char line[80];
    assert(local_parsed_array != 0);
    *parsed_array = local_parsed_array;
    long long counter;
    long data;
    fr = fopen (file_name, "rt");
    if(fr == 0)
    {
        printf("unable to open file %s\n", file_name);
        exit(2);
    }
    assert(fr != 0);
    for(counter = 0; counter < input_data_length; counter++)
    {
        assert(fgets(line, 80, fr) != 0);
        assert(sscanf(line, "%ld", &data) != 0);
        assert(data >= 0 && data < number_buckets);
        local_parsed_array[counter] = (unsigned char) data;
    }
    fclose(fr);
}

void print_histogram(int number_buckets, long long * histogram)
{
    int counter;
    for(counter = 0; counter < number_buckets; counter++)
    {
        printf("%d: %ld\n", counter, histogram[counter]);
    }
}

void usage(int argc, char * argv[])
{
    printf("Usage: %s INPUT_FILE INPUT_LENGTH MAX_INPUT_NUMBER NUMBER_THREADS\n", argv[0]);
    exit(2);
}

void print_time(sync_clock_t *sc)
{
    // Print out the time taken
    printf("### test took %1.31f seconds\n", (sc->t2.tv_sec - sc->t1.tv_sec)  + (float) (sc->t2.tv_nsec - sc->t1.tv_nsec) / 1000000000.0);
}
