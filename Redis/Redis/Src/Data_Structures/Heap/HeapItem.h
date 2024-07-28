#pragma once
#include <stdint.h>

class HeapItem 
{
public:
    uint64_t val = 0;
    size_t* ref = nullptr;
};