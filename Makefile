DEPFLAGS=-MT $@ -MMD -MP -MF $(patsubst %.o,%.Td,$@)
CXX=mpic++ -std=c++17 -msse -O3 -Wall -Werror -Wextra $(DEPFLAGS)
POSTCOMPILE=@mv -f $(patsubst %.o,%.Td,$@) $(patsubst %.o,%.d,$@) && touch $@
SRCS=gen/generator.cpp gen/main.cpp src/main.cpp src/main-playground.cpp src/main-bmark.cpp

all: bin/sn-gen bin/sn-mpi bin/sn-mpi-bmark bin/sn-playground

clean:
	rm -rf bin/ obj/ src/sn_sort.hpp

obj/%.d: ;

.PRECIOUS: obj/%.d

include $(patsubst %.cpp,obj/%.d,$(SRCS))

obj/src/%.o: src/%.cpp src/sn_sort.hpp
	@mkdir -p $(shell dirname "$@")
	$(CXX) -c -o $@ $<
	$(POSTCOMPILE)

obj/gen/%.o: gen/%.cpp
	@mkdir -p $(shell dirname "$@")
	$(CXX) -c -o $@ $<
	$(POSTCOMPILE)

bin/sn-gen: obj/gen/generator.o obj/gen/main.o
	@mkdir -p $(shell dirname "$@")
	$(CXX) -o $@ $^

src/sn_sort.hpp: bin/sn-gen
	$< 32 >$@

bin/sn-mpi: obj/src/main.o
	@mkdir -p $(shell dirname "$@")
	$(CXX) -o $@ $^

bin/sn-mpi-bmark: obj/src/main-bmark.o
	@mkdir -p $(shell dirname "$@")
	$(CXX) -o $@ $^ -lstdc++fs

bin/sn-playground: obj/src/main-playground.o
	@mkdir -p $(shell dirname "$@")
	$(CXX) -o $@ $^

data/report.csv: script/gather.sh
	$< > $@

data/plot.pdf: script/plot.R data/report.csv
	Rscript --vanilla $<

test: data/report.csv

define make-test

data/$(SZ0)G-$(N).log: bin/sn-mpi-bmark
	script/bmark-helper.sh -a -o $$@ -t $$$$(($(SZ0) / 17 + 1)) $(SZ0) $(N) -- -C skylake --contiguous

data/report.csv: data/$(SZ0)G-$(N).log

endef

$(foreach SZ0,16 32 64 128 256 512 1024 2048 4096 8192,$(foreach N,64 128 256 512,$(eval $(make-test))))
