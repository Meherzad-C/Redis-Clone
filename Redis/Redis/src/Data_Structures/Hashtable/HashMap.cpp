#include "HashMap.h"
#include <assert.h>

// ==============================
//	Constructors and Destructors
// ==============================

HMap::HMap() : resizing_pos(0) 
{

}

// ==============================
//	Public Member Functions
// ==============================

HNode* HMap::HM_Lookup(HNode* key, bool (*eq)(HNode*, HNode*)) 
{
    HM_HelpResizing();
    HNode** from = ht1.HT_Lookup(key, eq);

    if (!from) 
    {
        from = ht2.HT_Lookup(key, eq);
    }
    return from ? *from : nullptr;
}

void HMap::HM_Insert(HNode* node) 
{
    if (!ht1.table) 
    {
        ht1.HT_Init(4);
    }
    ht1.HT_Insert(node);

    if (!ht2.table) 
    {
        size_t load_factor = ht1.size / (ht1.mask + 1);
        if (load_factor >= k_max_load_factor)
        {
            HM_StartResizing();
        }
    }
    HM_HelpResizing();
}

HNode* HMap::HM_Pop(HNode* key, bool (*eq)(HNode*, HNode*))
{
    HM_HelpResizing();
    HNode** from = ht1.HT_Lookup(key, eq);
    if (from) 
    {
        return ht1.HT_Detach(from);
    }
    from = ht2.HT_Lookup(key, eq);
    if (from) 
    {
        return ht2.HT_Detach(from);
    }
    return nullptr;
}

size_t HMap::HM_Size() const
{
    return ht1.size + ht2.size;
}

bool HMap::HM_HNodeSame(HNode* lhs, HNode* rhs)
{
    return lhs == rhs;
}

void HMap::HM_Scan(HTableType ht, void(*f)(HNode*, void*), void* arg)
{
    if (ht == HTableType::PRIMARY_HT1)
    {
        ht1.HT_Scan(f, arg);
    }

    if (ht == HTableType::SECONDARY_HT2)
    {
        ht2.HT_Scan(f, arg);
    }
}

void HMap::Destroy() 
{
    ht1.~HTable();
    ht2.~HTable();
}

void HMap::HM_HelpResizing()
{
    size_t nwork = 0;
    while (nwork < k_resizing_work && ht2.size > 0) 
    {
        HNode** from = &ht2.table[resizing_pos];
        if (!*from) 
        {
            resizing_pos++;
            continue;
        }

        ht1.HT_Insert(ht2.HT_Detach(from));
        nwork++;
    }

    if (ht2.size == 0 && ht2.table) 
    {
        ht2.~HTable();
        ht2 = HTable();
    }
}

void HMap::HM_StartResizing()
{
    assert(ht2.table == nullptr);
    ht2 = ht1;
    ht1.HT_Init((ht1.mask + 1) * 2);
    resizing_pos = 0;
}
