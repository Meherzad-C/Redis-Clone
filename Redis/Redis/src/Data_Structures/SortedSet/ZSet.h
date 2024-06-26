#pragma once

#include <assert.h>
#include <string.h>
#include <stdlib.h>

// project includes
#include "..\AVLTree\AVLTree.h"
#include "..\Hashtable\HashMap.h"
#include "..\SortedSet\ZNode.h"
#include "..\..\Common\Utility.h"

class ZSet 
{
private:
    AVLNode* node = nullptr;
    HMap hmap;

private:
    static bool Zless(AVLNode* lhs, double score, const char* name, size_t len);
    static bool Zless(AVLNode* lhs, AVLNode* rhs);
    static bool ZHashCompare(HNode* node, HNode* key);

    void ZTree_Add(ZNode* node);
    void ZTree_Dispose(AVLNode* node);

public:
    bool Add(const char* name, size_t len, double score);
    ZNode* Lookup(const char* name, size_t len);
    ZNode* Pop(const char* name, size_t len);
    ZNode* Query(double score, const char* name, size_t len);
    void Dispose();
    static ZNode* Offset(ZNode* node, int64_t offset);
};