#include <pthread.h>
#include <time.h>

struct tinfo_t_
{
    pthread_t pth;
    struct sync_clock_t_ *sc;
    void (*entry)(int, void *);
    int nth;
    void *arg;
};

struct sync_clock_t_
{
    int total, started, running;
    pthread_cond_t before_cv, after_cv;
    pthread_mutex_t before_mp, after_mp;
    struct tinfo_t_ *tinfos;
    struct timespec t1, t2;
};

typedef struct tinfo_t_ tinfo_t;
typedef struct sync_clock_t_ sync_clock_t;

void check(int ret);

sync_clock_t *make_sync_clock(int nth);
void free_sync_clock(sync_clock_t *sc); // NOT responsible for tinfos[].arg
void run_all(sync_clock_t *sc, void (*entry)(int, void *), void *arg);

void load_data(char * file_name, long long input_data_length, unsigned char ** parsed_array, int number_buckets);
void compute_histogram_serial(long long ** output_data, unsigned char * input_data, long long input_data_length, int number_buckets, int number_threads);
void print_histogram(int number_buckets, long long * histogram);
void usage(int argc, char * argv[]);
void print_time(sync_clock_t *sc);
