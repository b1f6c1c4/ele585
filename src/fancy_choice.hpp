template <size_t I>
struct fancy_choice : fancy_choice<I-1> { };

template <>
struct fancy_choice<0> { };
