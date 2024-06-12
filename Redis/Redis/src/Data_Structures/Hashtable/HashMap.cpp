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

HNode* HMap::Lookup(HNode* key, bool (*eq)(HNode*, HNode*)) 
{
    HelpResizing();
    HNode** from = ht1.Lookup(key, eq);

    if (!from) 
    {
        from = ht2.Lookup(key, eq);
    }
    return from ? *from : nullptr;
}

void HMap::Insert(HNode* node) 
{
    if (!ht1.table) 
    {
        ht1.Init(4);
    }
    ht1.Insert(node);

    if (!ht2.table) 
    {
        size_t load_factor = ht1.size / (ht1.mask + 1);
        if (load_factor >= k_max_load_factor)
        {
            StartResizing();
        }
    }
    HelpResizing();
}

HNode* HMap::Pop(HNode* key, bool (*eq)(HNode*, HNode*)) 
{
    HelpResizing();
    HNode** from = ht1.Lookup(key, eq);
    if (from) 
    {
        return ht1.Detach(from);
    }
    from = ht2.Lookup(key, eq);
    if (from) 
    {
        return ht2.Detach(from);
    }
    return nullptr;
}

size_t HMap::Size() const 
{
    return ht1.size + ht2.size;
}

void HMap::Destroy() 
{
    ht1.~HTable();
    ht2.~HTable();
}

void HMap::HelpResizing() 
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

        ht1.Insert(ht2.Detach(from));
        nwork++;
    }

    if (ht2.size == 0 && ht2.table) 
    {
        ht2.~HTable();
        ht2 = HTable();
    }
}

void HMap::StartResizing() 
{
    assert(ht2.table == nullptr);
    ht2 = ht1;
    ht1.Init((ht1.mask + 1) * 2);
    resizing_pos = 0;
}
