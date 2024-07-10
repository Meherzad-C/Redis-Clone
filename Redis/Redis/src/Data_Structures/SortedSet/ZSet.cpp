#include "ZSet.h"

// ZSet static methods
bool ZSet::Zless(AVLNode* lhs, double score, const char* name, size_t len) 
{
    ZNode* zl = CONTAINER_OF(lhs, ZNode, node);
    if (zl->score != score) 
    {
        return zl->score < score;
    }
    int rv = memcmp(zl->name, name, _Min(zl->length, len));
    if (rv != 0) 
    {
        return rv < 0;
    }
    return zl->length < len;
}

bool ZSet::Zless(AVLNode* lhs, AVLNode* rhs) 
{
    ZNode* zr = CONTAINER_OF(rhs, ZNode, node);
    return Zless(lhs, zr->score, zr->name, zr->length);
}

bool ZSet::ZHashCompare(HNode* node, HNode* key) 
{
    ZNode* znode = CONTAINER_OF(node, ZNode, hmap);
    HKey* hkey = CONTAINER_OF(key, HKey, node);
    if (znode->length != hkey->len) 
    {
        return false;
    }
    return 0 == memcmp(znode->name, hkey->name, znode->length);
}

// ZSet methods
void ZSet::ZTree_Add(ZNode* node) 
{
    AVLNode* cur = nullptr;
    AVLNode** from = &(this->node);
    while (*from) 
    {
        cur = *from;
        from = Zless(&node->node, cur) ? &cur->left : &cur->right;
    }
    *from = &node->node;
    node->node.parent = cur;
    this->node = node->tree->AvlT_Fix(&node->node);
}

void ZSet::ZTree_Dispose(AVLNode* node) 
{
    if (!node) 
    {
        return;
    }
    ZTree_Dispose(node->left);
    ZTree_Dispose(node->right);
    ZNode::Destroy(CONTAINER_OF(node, ZNode, node));
}

bool ZSet::Add(const char* name, size_t len, double score) 
{
    ZNode* node = Lookup(name, len);
    if (node) 
    {
        if (node->score == score) 
        {
            return false;
        }
        this->node = node->tree->AvlT_Delete(&node->node);
        node->score = score;
        node->tree->Init(&node->node);
        ZTree_Add(node);
        return false;
    }
    else 
    {
        node = ZNode::Create(name, len, score);
        hmap.HM_Insert(&node->hmap);
        ZTree_Add(node);
        return true;
    }
}

ZNode* ZSet::Lookup(const char* name, size_t len) 
{
    if (!this->node) 
    {
        return nullptr;
    }

    HKey key;
    key.node.hcode = StringHash((uint8_t*)name, len);
    key.name = name;
    key.len = len;
    HNode* found = hmap.HM_Lookup(&key.node, &ZHashCompare);
    return found ? CONTAINER_OF(found, ZNode, hmap) : nullptr;
}

ZNode* ZSet::Pop(const char* name, size_t len) 
{
    if (!this->node)
    {
        return nullptr;
    }

    HKey key;
    key.node.hcode = StringHash((uint8_t*)name, len);
    key.name = name;
    key.len = len;
    HNode* found = hmap.HM_Pop(&key.node, &ZHashCompare);
    if (!found) 
    {
        return nullptr;
    }

    ZNode* node = CONTAINER_OF(found, ZNode, hmap);
    this->node = node->tree->AvlT_Delete(&node->node);
    return node;
}

ZNode* ZSet::Query(double score, const char* name, size_t len) 
{
    AVLNode* found = nullptr;
    AVLNode* cur = this->node;
    while (cur) 
    {
        if (Zless(cur, score, name, len)) 
        {
            cur = cur->right;
        }
        else 
        {
            found = cur;
            cur = cur->left;
        }
    }
    return found ? CONTAINER_OF(found, ZNode, node) : nullptr;
}

ZNode* ZSet::Offset(ZNode* node, int64_t offset) 
{
    AVLNode* tnode = node ? node->tree->Avlt_Offset(&node->node, offset) : nullptr;
    return tnode ? CONTAINER_OF(tnode, ZNode, node) : nullptr;
}

void ZSet::Dispose() 
{
    ZTree_Dispose(this->node);
    hmap.Destroy();
}