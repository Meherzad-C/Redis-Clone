#pragma once

#include "HashTable.h"

// The real hashtable interface.
// It uses 2 hashtables for progressive resizing.
class HMap 
{
private:
    static const size_t k_resizing_work = 128;
    static const size_t k_max_load_factor = 8;

    HTable ht1; // newer
    HTable ht2; // older
    size_t resizing_pos;
    
public:
    enum class HTableType
    {
        PRIMARY_HT1,
        SECONDARY_HT2
    };

public:
    HMap();
    HNode* HM_Lookup(HNode* key, bool (*eq)(HNode*, HNode*));
    void HM_Insert(HNode* node);
    HNode* HM_Pop(HNode* key, bool (*eq)(HNode*, HNode*));
    size_t HM_Size() const;
    bool HM_HNodeSame(HNode* lhs, HNode* rhs);
    void HM_Scan(HTableType ht, void (*f)(HNode*, void*), void* arg);
    void HM_HelpResizing();
    void HM_StartResizing();
    void Destroy();
};
