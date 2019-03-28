CFLAGS=-lrt
CPU=8

.PRECIOUS: bin/serial/histogram

all: bin/serial/histogram bin/parallel/A bin/parallel/B bin/parallel/C bin/parallel/D bin/parallel/E bin/parallel_mpi/F bin/parallel_mpi/G

bin/serial/%: src/serial/%.c
	mkdir -p $(shell dirname $@)
	icc -O3 -o $@ $^ $(CFLAGS)

bin/parallel/%: src/parallel/%.c src/parallel/util.c src/parallel/util.h
	mkdir -p $(shell dirname $@)
	icc -O3 -pthread -o $@ $^ $(CFLAGS)

bin/parallel_mpi/%: src/parallel_mpi/%.c
	mkdir -p $(shell dirname $@)
	mpicc -O3 -o $@ $^ -limf -lm $(CFLAGS)

clean:
	rm -rf bin/ data/standard/ data/parallel/ data/parallel_mpi/

define make-report

data/report/$(X).csv:
	mkdir -p $$(shell dirname $$@)
	grep -Po '\d\.\d+' $$^ | sed 's_^data/parallel/./__; s_/_,_g; s_\.txt:_,_' > $$@

test-$(X): data/report/$(X).csv

test: test-$(X)

endef

$(foreach X,A B C D E,$(eval $(make-report)))

define make-debug

debug-$(X)-$(NT): bin/parallel/$(X) data/input/128.txt
	$$^ 128 255 $(NT)

endef

$(foreach NT,1 2 4 8 16 32 64,$(foreach X,A B C D E,$(eval $(make-debug))))

define make-parallel

data/parallel/$(X)/balanced/$(NT)/$(N).txt: bin/parallel/$(X) data/input/$(N).txt data/standard/balanced/$(N).txt
	mkdir -p $$(shell dirname $$@)
	salloc -N 1 -n 1 --cpus-per-task=$(CPU) -t 4:00:00 -J $(X)-b$(NT)-$(N) \
		srun -o $$@ ./run.sh $(X) $(NT) $(N) 255

data/parallel/$(X)/unbalanced/$(NT)/$(N).txt: bin/parallel/$(X) data/input/$(N).txt data/standard/unbalanced/$(N).txt
	mkdir -p $$(shell dirname $$@)
	salloc -N 1 -n 1 --cpus-per-task=$(CPU) -t 4:00:00 -J $(X)-u$(NT)-$(N) \
		srun -o $$@ ./run.sh $(X) $(NT) $(N) 511

data/report/$(X).csv: data/parallel/$(X)/balanced/$(NT)/$(N).txt data/parallel/$(X)/unbalanced/$(NT)/$(N).txt

endef

define make-input

.PRECIOUS: data/input/$(N).txt

input: data/input/$(N).txt

standard: data/standard/balanced/$(N).txt data/standard/unbalanced/$(N).txt

data/input/$(N).txt:
	mkdir -p $$(shell dirname $$@)
	./gen_random_numbers.py --seed 42 --ofile $$@ --number $(N) --max 255

data/standard/balanced/$(N).txt: bin/serial/histogram data/input/$(N).txt
	mkdir -p $$(shell dirname $$@)
	$$^ $(N) 255 1 > $$@
	sed -i '$$$$ d' $$@

data/standard/unbalanced/$(N).txt: bin/serial/histogram data/input/$(N).txt
	mkdir -p $$(shell dirname $$@)
	$$^ $(N) 511 1 > $$@
	sed -i '$$$$ d' $$@

$$(foreach NT,1 2 4 8 16 32 64,$$(foreach X,A B C D E,$$(eval $$(make-parallel))))

endef

$(foreach N,128 8192 524288 33554432,$(eval $(make-input)))

# bin/run_4_parallel_mpi_E_histogram: parallel_mpi_E_histogram input_data_1024
# 	mpiexec -npernode 4 --mca btl ^openib ./parallel_mpi_E_histogram input_data_1024 1024 255 1
