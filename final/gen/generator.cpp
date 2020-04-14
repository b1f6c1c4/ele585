#include "generator.h"
#include <algorithm>
#include <iostream>

#define P(l, r) G.push(std::make_pair(l, r))
#define Y(...) G.merge(__VA_ARGS__)
#define X(l, r) Y(sorting_network(2, ((r) - (l)) * NS, B + (l) * NS, NT))
sorting_network_generator::gen_t sorting_network_generator::sorting_network(RNG())
{
	gen_t G{};
    switch (N)
    {
		case 0:
		case 1:
			break;
        case 2:
            if (B + NS < NT)
                P(B, B + NS);
            break;
        case 3:
            X(0, 1), X(1, 2), X(0, 1);
            break;
        case 9:
            X(0, 1), X(1, 2), X(0, 1);
            X(3, 4), X(4, 5), X(3, 4);
            X(6, 7), X(7, 8), X(6, 7);
            X(0, 3), X(3, 6), X(0, 3);
            X(1, 4), X(4, 7), X(1, 4);
            X(2, 5), X(5, 8), X(2, 5);
            X(1, 3), X(5, 7);
            X(2, 6), X(4, 6), X(2, 4);
            X(2, 3), X(5, 6);
            break;
        case 10:
            X(0, 5), X(1, 6), X(2, 7), X(3, 8), X(4, 9);
            X(0, 3), X(1, 4), X(5, 8), X(6, 9);
            X(0, 2), X(3, 6), X(7, 9);
            X(0, 1), X(2, 4), X(5, 7), X(8, 9);
            X(1, 2), X(3, 5), X(4, 6), X(7, 8);
            X(1, 3), X(2, 5), X(4, 7), X(6, 8);
            X(2, 3), X(3, 4), X(6, 7), X(5, 6), X(4, 5);
            break;
        case 12:
            Y(sorting_network(4, NS, B + (0 * NS), NT));
            Y(sorting_network(4, NS, B + (4 * NS), NT));
            Y(sorting_network(4, NS, B + (8 * NS), NT));
            X(0, 4), X(4, 8), X(0, 4), X(1, 5), X(5, 9), X(1, 5);
            X(6, 10), X(2, 6), X(6, 10), X(7, 11), X(3, 7), X(7, 11);
            X(1, 4), X(7, 10), X(3, 8);
            X(2, 3), X(2, 4), X(3, 5), X(8, 9), X(7, 9), X(6, 8);
            X(3, 4), X(5, 6), X(7, 8);
            break;
        case 13:
            X(0, 5), X(1, 7), X(3, 9), X(2, 4), X(6, 11), X(8, 12);
            X(0, 6), X(1, 3), X(2, 8), X(4, 12), X(5, 11), X(7, 9);
            X(0, 2), X(4, 5), X(6, 8), X(9, 10), X(11, 12);
            X(3, 11), X(5, 9), X(7, 8), X(10, 12);
            X(1, 5), X(2, 3), X(4, 7), X(8, 10), X(9, 11);
            X(0, 1), X(5, 6), X(8, 9), X(10, 11);
            X(1, 4), X(2, 5), X(3, 6), X(7, 8), X(9, 10);
            X(1, 2), X(3, 7), X(4, 5), X(6, 8);
            X(2, 4), X(3, 5), X(6, 7), X(8, 9);
            X(3, 4), X(5, 6);
            break;
        case 16:
#define STAGE(x) X(x + 0, x + 1), X(x + 2, x + 3), X(x + 0, x + 2), X(x + 1, x + 3)
            STAGE(0), STAGE(4), STAGE(8), STAGE(12);
#undef STAGE
#define STAGE(x) X(x + 0, x + 4), X(x + 1, x + 5), X(x + 2, x + 6), X(x + 3, x + 7)
            STAGE(0), STAGE(8);
#undef STAGE
            X(0, 8), X(1, 9), X(2, 10), X(3, 11), X(4, 12), X(5, 13), X(6, 14), X(7, 15);
            X(1, 2), X(3, 12), X(4, 8), X(5, 10), X(6, 9), X(7, 11), X(13, 14);
            X(1, 4), X(2, 8), X(7, 13), X(11, 14);
            X(2, 4), X(3, 8), X(5, 6), X(7, 12), X(9, 10), X(11, 13);
            X(3, 5), X(6, 8), X(7, 9), X(10, 12);
            X(3, 4), X(5, 6), X(7, 8), X(9, 10), X(11, 12);
            X(6, 7), X(8, 9);
            break;
        case 11:
        case 14:
        case 15:
            Y(sorting_network(N + 1, NS, B, NT));
			break;
        default:
            auto P = ceil_2(N), Q = floor_2(N);
#define T(n, ns, b) (n), (ns), (b), std::min(NT, (b) + (n) * (ns))
#define L T(P, NS, B + (0 * NS))
#define R T(Q, NS, B + (P * NS))
            Y(sorting_network(L));
			Y(sorting_network(R));
            Y(odd_even(L, R));
			break;
    }
	return G;
}

sorting_network_generator::gen_t sorting_network_generator::odd_even(RNG(a), RNG(b))
{
	gen_t G{};
	if (Na == 0 || Nb == 0)
		return G;
	if (Na == 1 && Nb == 1)
	{
		if (Ba < NTa && Bb < NTb)
			P(Ba, Bb);
		return G;
	}

#define Ta(n, ns, b) (n), (ns), (b), std::min(NTa, (b) + (n) * (ns) + 1)
#define Tb(n, ns, b) (n), (ns), (b), std::min(NTb, (b) + (n) * (ns) + 1)
    Y(odd_even(
        Ta(ceil_2(Na),  2 * NSa, Ba + 0 * NSa),
        Tb(ceil_2(Nb),  2 * NSb, Bb + 0 * NSb)
        ),
    odd_even(
        Ta(floor_2(Na), 2 * NSa, Ba + 1 * NSa),
        Tb(floor_2(Nb), 2 * NSb, Bb + 1 * NSb)
        ));

    auto valid = false;
    size_t last = 0;

    for (size_t i = 1; i < Na; i++)
    {
        if (!valid)
            valid = true, last = Ba + i * NSa;
        else
            valid = false, P(last, Ba + i * NSa);
    }

    for (size_t i = 0; i < Nb; i++)
    {
        if (!valid)
            valid = true, last = Bb + i * NSb;
        else if (last < NTb && Bb + i * NSb < NTb)
            valid = false, P(last, Bb + i * NSb);
    }

	return G;
}
