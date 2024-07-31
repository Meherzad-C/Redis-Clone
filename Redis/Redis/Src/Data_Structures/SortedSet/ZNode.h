#pragma once

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

// project includes
#include "..\AVLTree\AVLTree.h"
#include "..\Hashtable\HashMap.h"
#include "..\..\Common\Utility.h"

struct HKey 
{
    HNode node;
    const char* name = nullptr;
    size_t len = 0;
};

// ZNode for ZSet
class ZNode 
{
public:
    AVLNode node;
    AVLTree* tree;
    HNode hmap;
    double score = 0;
    size_t length = 0;
    char name[0];

public:
    static ZNode* Create(const char* name, size_t len, double score);
    static void Destroy(ZNode* node);
};