CFLAGS = -lrt

.PRECIOUS: bin/serial/histogram

all: bin/serial/histogram bin/parallel/A bin/parallel/B bin/parallel/C bin/parallel/D bin/parallel/E bin/parallel_mpi/F bin/parallel_mpi/G

bin/serial/%: src/serial/%.c
	mkdir -p $(shell dirname $@)
	icc -O3 -o $@ $^ $(CFLAGS)

bin/parallel/%: src/parallel/%.c
	mkdir -p $(shell dirname $@)
	icc -O3 -pthread -o $@ $^ $(CFLAGS)

bin/parallel_mpi/%: src/parallel_mpi/%.c
	mkdir -p $(shell dirname $@)
	mpicc -O3 -o $@ $^ -limf -lm $(CFLAGS)

# bin/run_4_parallel_mpi_E_histogram: parallel_mpi_E_histogram input_data_1024
# 	mpiexec -npernode 4 --mca btl ^openib ./parallel_mpi_E_histogram input_data_1024 1024 255 1

clean:
	rm -rf bin/ data/standard/ data/parallel/ data/parallel_mpi/

define make-parallel

data/parallel/$(X)/$(NT)/$(N).txt: bin/parallel/$(X) data/input/$(N).txt
	mkdir -p $$(shell dirname $$@)
	$$^ $(N) 255 $(NT) > $$@
	[ -z "$$$$(diff $$@ data/input/$(N).txt)" ]

test-$(X): data/parallel/$(X)/$(NT)/$(N).txt

test: test-$(X)

endef

define make-input

.PRECIOUS: data/input/$(N).txt

input: data/input/$(N).txt

standard: data/standard/$(N).txt

data/input/$(N).txt:
	mkdir -p $$(shell dirname $$@)
	./gen_random_numbers.py --seed 42 --ofile $$@ --number $(N) --max 255

data/standard/$(N).txt: bin/serial/histogram data/input/$(N).txt
	mkdir -p $$(shell dirname $$@)
	$$^ $(N) 255 1 > $$@

$$(foreach NT,1 2 4 8 16 32 64,$$(foreach X,A B C D E,$$(eval $$(make-parallel))))

endef

$(foreach N,128 8192 524288 33554432,$(eval $(make-input)))
