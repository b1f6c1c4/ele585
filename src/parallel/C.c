#include <stdio.h>
#include <stdlib.h>
#include "util.h"

typedef struct
{
    int nth;
    unsigned char *src;
    int nsrc;
    long long *dest;
    int ndest;
} info_t;

void entry(int ith, void *info_)
{
    info_t *info = (info_t *)(info_);
    unsigned char ndest = info->ndest / info->nth;
    unsigned char lb = ndest * ith;
    unsigned char *src = info->src;
    int nsrc = info->nsrc;

    unsigned char v;
    while (nsrc--)
    {
        v = *src++;
        if (info->nth != 1)
            if (v < lb || v >= lb + ndest)
                continue;
        info->dest[v]++;
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
}
