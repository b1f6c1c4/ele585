#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

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
    printf("Usage: %s INPUT_FILE INPUT_LENGTH MAX_INPUT_NUMBER NUMBER_THREADS\n", argv[0]);
    exit(2);
}

int main(int argc, char * argv[], char**envp)
{
    unsigned char * data_array;
    long long * histogram_array;
    struct timespec res, t1, t2;
    if(argc != 5)
    {
        usage(argc, argv);
    }
    assert (atoi(argv[4]) == 1);
    // FIXME, should have better error checking on these atoi's as atoi has
    // bad error handling
    load_data(argv[1], atoi(argv[2]), &data_array, atoi(argv[3])+1);
    // Get the starting time
    clock_gettime(CLOCK_REALTIME, &t1);
    compute_histogram_serial(&histogram_array, data_array, atoi(argv[2]), atoi(argv[3])+1, atoi(argv[4]));
    // Get the ending time
    clock_gettime(CLOCK_REALTIME, &t2);
    print_histogram(atoi(argv[3])+1, histogram_array);
    // Print out the time taken
    printf("### test took %1.31f seconds\n", (t2.tv_sec - t1.tv_sec)  + (float) (t2.tv_nsec - t1.tv_nsec) / 1000000000.0);
}
