#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include <mpi.h>

int myid = -1;

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

void compute_histogram_serial(long long ** output_data, unsigned char * input_data, long long input_data_length, int number_buckets, int number_threads)
{
    long long * local_output_data = malloc(sizeof(long long) * number_buckets);
    assert(local_output_data != 0);
    *output_data = local_output_data;
    int counter_local;
    for(counter_local = 0; counter_local < number_buckets; counter_local++)
    {
        local_output_data[counter_local] = 0;
    }
    long long counter;
    for(counter = 0; counter < input_data_length; counter++)
    {
        local_output_data[input_data[counter]]++;
    }
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
    if(myid == 0)
    {
        printf("Usage: %s INPUT_FILE INPUT_LENGTH MAX_INPUT_NUMBER NUMBER_THREADS\n", argv[0]);
    }
    MPI_Finalize();
    exit(2);
}

#define TAG_RESULT 0xdead

void entry(int ith, int nth, unsigned char *src_, int nsrc_, long long *dest, int ndest)
{
    int nsrc = nsrc_ / nth;
    unsigned char *src = src_ + nsrc * ith;
    int ext = nsrc_ - nsrc * nth;
    if (ith >= nth - ext)
        src += ith - (nth - ext), nsrc++;

    while (nsrc--)
        dest[*src++]++;
}

int main(int argc, char * argv[], char**envp)
{
    struct timespec t1, t2;
    unsigned char * data_array;
    long long * histogram_array;
    if(argc != 5)
    {
        usage(argc, argv);
    }

    long long nin = atoi(argv[2]);
    int nbkt = atoi(argv[3])+1;
    int nth = atoi(argv[4]);
    assert(nth == 1);

    load_data(argv[1], nin, &data_array, nbkt);
    histogram_array = calloc(nbkt, sizeof(long long));

    int i, j;
    int number_nodes;
    MPI_Status ret;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &number_nodes);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    int rx, tx;

    if(myid == 0)
    {
        long long *rx = malloc(nbkt * sizeof(long long));
        MPI_Barrier(MPI_COMM_WORLD);

        clock_gettime(CLOCK_REALTIME, &t1);

        entry(myid, number_nodes, data_array, nin, histogram_array, nbkt);

        for (i = 1; i < number_nodes; i++)
        {
            MPI_Recv(rx, nbkt, MPI_LONG_LONG_INT, MPI_ANY_SOURCE, TAG_RESULT, MPI_COMM_WORLD, &ret);
            for (j = 0; j < nbkt; j++)
                histogram_array[j] += rx[j];
        }

        clock_gettime(CLOCK_REALTIME, &t2);
        free(rx);

        print_histogram(nbkt, histogram_array);
        printf("### test took %1.31f seconds\n", (t2.tv_sec - t1.tv_sec)  + (float) (t2.tv_nsec - t1.tv_nsec) / 1000000000.0);

        MPI_Barrier(MPI_COMM_WORLD);
    }
    else
    {
        MPI_Barrier(MPI_COMM_WORLD);

        entry(myid, number_nodes, data_array, nin, histogram_array, nbkt);

        MPI_Send(histogram_array, nbkt, MPI_LONG_LONG_INT, 0, TAG_RESULT, MPI_COMM_WORLD);

        MPI_Barrier(MPI_COMM_WORLD);
    }

    MPI_Finalize();
}
