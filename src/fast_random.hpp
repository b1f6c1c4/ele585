#pragma once

#include <utility>
#include <algorithm>
#include <emmintrin.h>

class fast_random
{
public:
	fast_random(unsigned int seed)
		: cur_seed(_mm_set_epi32(seed, seed+1, seed, seed+1)) { }

	template <typename T>
	T operator()()
	{
		const size_t sz0 = std::max(static_cast<size_t>(4), sizeof(T) / sizeof(unsigned int));
		const size_t sz = (sz0 & 3) ? (sz0 - (sz0 & 3) + 4) : sz0;
		unsigned int buffer[sz];

		for (auto ptr = buffer; ptr < buffer + sz; ptr += 4)
			rand_sse(ptr);

		T t;
		std::copy(
			reinterpret_cast<const char *>(buffer),
			reinterpret_cast<const char *>(buffer) + sizeof(T),
			reinterpret_cast<char *>(&t));
		return t;
	}

	template <typename T>
	void operator()(T *d, size_t sz)
	{
		const auto bytes = sizeof(T) * sz;
		if (bytes % (4 * sizeof(unsigned int)) == 0)
		{
			for (
				auto ptr = reinterpret_cast<unsigned int *>(d);
				ptr < reinterpret_cast<unsigned int *>(d + sz);
				ptr += 4)
				rand_sse(ptr);
		}
		else
		{
			for (size_t i = 0; i < sz; i++)
				d[i] = operator()<T>();
		}
	}

private:
	__attribute__ ((aligned(16))) __m128i cur_seed;
	__attribute__ ((aligned(16))) unsigned int mult[4] =
		{ 214013, 17405, 214013, 69069 };
	__attribute__ ((aligned(16))) unsigned int gadd[4] =
		{ 2531011, 10395331, 13737667, 1 };
	__attribute__ ((aligned(16))) unsigned int mask[4] =
		{ 0xFFFFFFFF, 0, 0xFFFFFFFF, 0 };
	__attribute__ ((aligned(16))) unsigned int masklo[4] =
		{ 0x00007FFF, 0x00007FFF, 0x00007FFF, 0x00007FFF };

	void rand_sse(unsigned int *result)
	{
		__attribute__ ((aligned(16))) __m128i cur_seed_split;
		__attribute__ ((aligned(16))) __m128i multiplier;
		__attribute__ ((aligned(16))) __m128i adder;
		__attribute__ ((aligned(16))) __m128i mod_mask;

		adder = _mm_load_si128((__m128i*) gadd);
		multiplier = _mm_load_si128((__m128i*) mult);
		mod_mask = _mm_load_si128((__m128i*) mask);
		cur_seed_split = _mm_shuffle_epi32(cur_seed, _MM_SHUFFLE(2, 3, 0, 1));
		cur_seed = _mm_mul_epu32(cur_seed, multiplier);
		multiplier = _mm_shuffle_epi32(multiplier, _MM_SHUFFLE(2, 3, 0, 1));
		cur_seed_split = _mm_mul_epu32(cur_seed_split, multiplier);
		cur_seed = _mm_and_si128(cur_seed, mod_mask);
		cur_seed_split = _mm_and_si128(cur_seed_split, mod_mask);
		cur_seed_split = _mm_shuffle_epi32(cur_seed_split, _MM_SHUFFLE(2, 3, 0, 1));
		cur_seed = _mm_or_si128(cur_seed, cur_seed_split);
		cur_seed = _mm_add_epi32(cur_seed, adder);
		_mm_storeu_si128((__m128i*) result, cur_seed);
		return;
	}
};
