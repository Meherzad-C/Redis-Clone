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

void HTable::HT_Init(size_t n) 
{
    assert(n > 0 && ((n - 1) & n) == 0);
    table = (HNode**)calloc(n, sizeof(HNode*));
    mask = n - 1;
    size = 0;
}

void HTable::HT_Insert(HNode* node) 
{
    size_t pos = node->hcode & mask;
    node->next = table[pos];
    table[pos] = node;
    size++;
}

HNode** HTable::HT_Lookup(HNode* key, bool (*eq)(HNode*, HNode*)) 
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

HNode* HTable::HT_Detach(HNode** from) 
{
    HNode* node = *from;
    *from = node->next;
    size--;
    return node;
}

void HTable::HT_Scan(void(*f)(HNode*, void*), void* arg)
{
    if (size == 0)
    {
        return;
    }
    for (size_t i = 0; i < mask + 1; ++i)
    {
        HNode* node = table[i];
        while (node)
        {
            f(node, arg);
            node = node->next;
        }
    }
}

HTable::~HTable() 
{
    free(table);
}
