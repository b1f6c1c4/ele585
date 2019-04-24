DEPFLAGS=-MT $@ -MMD -MP -MF obj/$*.Td
CXX=g++ -std=c++17 -O3 -Wall -Werror -Wextra $(DEPFLAGS)
POSTCOMPILE=@mv -f obj/$*.Td obj/$*.d && touch $@
SRCS=gen/generator.cpp gen/main.cpp src/main.cpp

all: bin/sn-gen bin/sn-mpi

clean:
	rm -rf bin/ obj/ src/sn.cpp

obj/%.d: ;

.PRECIOUS: obj/%.d

include $(wildcard $(patsubst %,obj/%.d,$(basename $(SRCS))))

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
