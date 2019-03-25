#include <stdio.h>
#include <stdlib.h>
#include "util.h"

typedef struct
{
    int data;
} info_t;

void entry(int nth, void *info_)
{
    info_t *info = (info_t *)(info_);
    printf("I'm %d of %d\n", nth, info->data);
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

    info_t info;
    info.data = 233;

    sync_clock_t *sc = make_sync_clock(nth);
    spawn_all(sc, &entry, &info);
    start_executions(sc);
    // compute_histogram_serial(&histogram_array, data_array, nin, nbkt, nth);
    wait_all_executions(sc);

    // print_histogram(nbkt, histogram_array);
    print_time(sc);
    free_sync_clock(sc);
}
