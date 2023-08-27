#include "iostream"
#include "kqnet.h"

enum msgids : uint8_t
{
    Transmitted,
    Received
};

uint64_t scramble(uint64_t input)
{
    auto out = input ^ 0x5A9B6C2F0F011;
    out = (out & 0xF0F0F0F0F0F0F0) >> 4;
    return out;
}