#include "HashTable.h"
#include <assert.h>

// ==============================
//	Constructors and Destructors
// ==============================

HTable::HTable() : table(nullptr), mask(0), size(0) 
{

}

// ==============================
//	Public Member Functions
// ==============================

void HTable::Init(size_t n) 
{
    assert(n > 0 && ((n - 1) & n) == 0);
    table = (HNode**)calloc(n, sizeof(HNode*));
    mask = n - 1;
    size = 0;
}

void HTable::Insert(HNode* node) 
{
    size_t pos = node->hcode & mask;
    node->next = table[pos];
    table[pos] = node;
    size++;
}

HNode** HTable::Lookup(HNode* key, bool (*eq)(HNode*, HNode*)) 
{
    if (!table) return nullptr;

    size_t pos = key->hcode & mask;
    HNode** from = &table[pos];
    for (HNode* cur; (cur = *from) != nullptr; from = &cur->next) 
    {
        if (cur->hcode == key->hcode && eq(cur, key)) 
        {
            return from;
        }
    }
    return nullptr;
}

HNode* HTable::Detach(HNode** from) 
{
    HNode* node = *from;
    *from = node->next;
    size--;
    return node;
}

HTable::~HTable() 
{
    free(table);
}
