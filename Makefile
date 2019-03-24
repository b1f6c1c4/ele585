CFLAGS = -lrt

all: serial_histogram parallel_A_histogram parallel_B_histogram parallel_C_histogram parallel_D_histogram parallel_mpi_E_histogram parallel_mpi_F_histogram

serial_histogram: histogram.c
	icc -O3 -o $@ $^ $(CFLAGS)

parallel_A_histogram: parallel_A_histogram.c
	icc -O3 -pthread -o $@ $^ $(CFLAGS)

parallel_B_histogram: parallel_B_histogram.c
	icc -pthread -O3 -o $@ $^ $(CFLAGS)

parallel_C_histogram: parallel_C_histogram.c
	icc -pthread -O3 -o $@ $^ $(CFLAGS)

parallel_D_histogram: parallel_D_histogram.c
	icc -pthread -O3 -o $@ $^ $(CFLAGS)

parallel_mpi_E_histogram: parallel_mpi_E_histogram.c
	mpicc -O3 -o $@ $^ -limf -lm $(CFLAGS)

parallel_mpi_F_histogram: parallel_mpi_F_histogram.c
	mpicc -O3 -o $@ $^ -limf -lm $(CFLAGS)

input_data_1024:
	./gen_random_numbers.py --seed 42 --ofile input_data_1024 --number 1024 --max 255

run_4_parallel_mpi_E_histogram: parallel_mpi_E_histogram input_data_1024
	mpiexec -npernode 4 --mca btl ^openib ./parallel_mpi_E_histogram input_data_1024 1024 255 1

clean:
	rm -f serial_histogram parallel_A_histogram parallel_B_histogram parallel_C_histogram parallel_D_histogram parallel_mpi_E_histogram parallel_mpi_F_histogram input_data_1024
