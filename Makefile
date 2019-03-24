CFLAGS = -lrt

.PRECIOUS: bin/serial/histogram

bin/serial/%: src/serial/%.c
	mkdir -p $(shell dirname $@)
	icc -O3 -o $@ $^ $(CFLAGS)

bin/parallel/%: src/parallel/%.c
	mkdir -p $(shell dirname $@)
	icc -O3 -pthread -o $@ $^ $(CFLAGS)

bin/parallel_mpi/%: src/parallel_mpi/%.c
	mkdir -p $(shell dirname $@)
	mpicc -O3 -o $@ $^ -limf -lm $(CFLAGS)

input: data/input/128.txt data/input/8192.txt data/input/524288.txt data/input/33554432.txt

standard: data/standard/128.txt data/standard/8192.txt data/standard/524288.txt data/standard/33554432.txt

data/input/%.txt:
	mkdir -p $(shell dirname $@)
	./gen_random_numbers.py --seed 42 --ofile $@ --number $(shell basename -s .txt $@) --max 255

data/standard/%.txt: bin/serial/histogram data/input/%.txt
	mkdir -p $(shell dirname $@)
	$^ $(shell basename -s .txt $@) 255 1 > $@

# bin/run_4_parallel_mpi_E_histogram: parallel_mpi_E_histogram input_data_1024
# 	mpiexec -npernode 4 --mca btl ^openib ./parallel_mpi_E_histogram input_data_1024 1024 255 1

clean:
	rm -rf bin/ data/standard/ data/parallel/ data/parallel_mpi/
