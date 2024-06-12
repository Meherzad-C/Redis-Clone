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
    void Init(size_t n);
    void Insert(HNode* node);
    HNode** Lookup(HNode* key, bool (*eq)(HNode*, HNode*));
    HNode* Detach(HNode** from);
    ~HTable();
};
