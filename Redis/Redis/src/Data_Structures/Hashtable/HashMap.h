#pragma once

#include "HashTable.h"

// the real hashtable interface.
// it uses 2 hashtables for progressive resizing.
class HMap 
{
private:
    static const size_t k_resizing_work = 128;
    static const size_t k_max_load_factor = 8;

    HTable ht1; // newer
    HTable ht2; // older
    size_t resizing_pos;

public:
    HMap();
    HNode* Lookup(HNode* key, bool (*eq)(HNode*, HNode*));
    void Insert(HNode* node);
    HNode* Pop(HNode* key, bool (*eq)(HNode*, HNode*));
    size_t Size() const;
    void Destroy();

    void HelpResizing();
    void StartResizing();
};
