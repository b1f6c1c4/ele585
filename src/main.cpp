#include "main.h"

#include <iostream>

extern const char *__progname;

int main(int argc, char *argv[])
{
    std::cout << __progname;
    return 0;
}
