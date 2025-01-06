#pragma once

#include <string>
#include <cstdio>

typedef bool boolean;
typedef uint8_t byte;

struct SerialWrapper
{
    static void print(double value)
    {
        printf("%f", value);
    }

    static void println(double value)
    {
        printf("%f\n", value);
    }

    static void print(int value)
    {
        printf("%i", value);
    }

    static void println(int value)
    {
        printf("%i\n", value);
    }

    static void print(uint32_t value)
    {
        printf("%lu", value);
    }

    static void println(uint32_t value)
    {
        printf("%lu\n", value);
    }

    static void println(std::string toPrint)
    {
        printf("%s\n", toPrint.c_str());
    }

    static void print(std::string toPrint)
    {
        printf("%s", toPrint.c_str());
    }
};

extern SerialWrapper Serial;

#define F(x) x

void delay(size_t ms);