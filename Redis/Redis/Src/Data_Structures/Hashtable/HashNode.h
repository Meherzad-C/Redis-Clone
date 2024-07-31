#pragma once

#include <stdint.h>

// Hashtable node. It will be embedded into the payload
class HNode 
{
public:
    HNode* next = nullptr;
    uint64_t hcode = 0;
};