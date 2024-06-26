#include "ZNode.h"

ZNode* ZNode::Create(const char* name, size_t len, double score) 
{
    ZNode* node = (ZNode*)malloc(sizeof(ZNode) + len);
    assert(node);
    node->tree.Init((&node->node));
    node->hmap.next = nullptr;
    node->hmap.hcode = StrHash((uint8_t*)name, len);
    node->score = score;
    node->length = len;
    memcpy(&node->name[0], name, len);
    return node;
}

void ZNode::Destroy(ZNode* node) 
{
    free(node);
}