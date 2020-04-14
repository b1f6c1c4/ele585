#include <stdio.h>
#include <stdlib.h>
#include "util.h"

typedef struct
{
    pthread_mutex_t *mus;
    int nth;
    unsigned char *src;
    int nsrc;
    long long *dest;
    int ndest;
} info_t;

void entry(int ith, void *info_)
{
    info_t *info = (info_t *)(info_);
    int nsrc = info->nsrc / info->nth;
    unsigned char *src = info->src + nsrc * ith;

    unsigned char v;
    while (nsrc--)
    {
        v = *src++;
        check(pthread_mutex_lock(&info->mus[v]));
        info->dest[v]++;
        check(pthread_mutex_unlock(&info->mus[v]));
    }
}

int main(int argc, char * argv[], char**envp)
{
    unsigned char * data_array;
    long long * histogram_array;
    if(argc != 5)
    {
        usage(argc, argv);
    }

    long long nin = atoi(argv[2]);
    int nbkt = atoi(argv[3])+1;
    int nth = atoi(argv[4]);

    load_data(argv[1], nin, &data_array, nbkt);
    histogram_array = calloc(nbkt, sizeof(long long));

    info_t info;
    int i;
    info.mus = malloc(nbkt * sizeof(pthread_mutex_t));
    for (i = 0; i < nbkt; i++)
        check(pthread_mutex_init(&info.mus[i], NULL));
    info.nth = nth;
    info.src = data_array;
    info.nsrc = nin;
    info.dest = histogram_array;
    info.ndest = nbkt;

    sync_clock_t *sc = make_sync_clock(nth);
    run_all(sc, &entry, &info);
    finalize(sc);

    print_histogram(nbkt, histogram_array);

    print_time(sc);
    free_sync_clock(sc);
    for (i = 0; i < nbkt; i++)
        check(pthread_mutex_destroy(&info.mus[i]));
    free(info.mus);
}
