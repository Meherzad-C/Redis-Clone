#pragma once

#include <stdint.h>

// hashtable node, will be embedded into the payload
class HNode 
{
public:
    HNode* next = nullptr;
    uint64_t hcode = 0;
};