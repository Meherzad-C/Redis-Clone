#pragma once

#include <stddef.h>
#include <stdlib.h>
#include "HashNode.h"

// a simple fixed-sized hashtable
class HTable 
{
public:
    HNode** table;
    size_t mask;
    size_t size;

public:
    HTable();
    void HT_Init(size_t n);
    void HT_Insert(HNode* node);
    HNode** HT_Lookup(HNode* key, bool (*eq)(HNode*, HNode*));
    HNode* HT_Detach(HNode** from);
    void HT_Scan(void (*f)(HNode*, void*), void* arg);
    ~HTable();
};
