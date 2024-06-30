//#include <assert.h>
//#include "..\Src\Data_Structures\AVLTree\AVLTree.h"
//
//
//#define CONTAINER_OF(ptr, type, member) \
//    (reinterpret_cast<type *>(reinterpret_cast<char *>(ptr) - offsetof(type, member)))
//
//
//struct Data 
//{
//    AVLNode node;
//    AVLTree tree;
//    uint32_t val = 0;
//};
//
//struct Container 
//{
//    AVLNode* root = NULL;
//};
//
//static void add(Container& c, uint32_t val, Data* data) 
//{
//    data = new Data();
//    data->tree.Init(&data->node);
//    data->val = val;
//
//    if (!c.root) 
//    {
//        c.root = &data->node;
//        return;
//    }
//
//    AVLNode* cur = c.root;
//    while (true) 
//    {
//        AVLNode** from =
//            (val < CONTAINER_OF(cur, Data, node)->val)
//            ? &cur->left : &cur->right;
//        if (!*from) 
//        {
//            *from = &data->node;
//            data->node.parent = cur;
//            c.root = data->tree.AvlT_Fix(&data->node);
//            break;
//        }
//        cur = *from;
//    }
//}
//
//static void dispose(AVLNode* node) 
//{
//    if (node) 
//    {
//        dispose(node->left);
//        dispose(node->right);
//        delete CONTAINER_OF(node, Data, node);
//    }
//}
//
//static void test_case(uint32_t sz) 
//{
//    Data* data{};
//    Container c;
//    for (uint32_t i = 0; i < sz; ++i) 
//    {
//        add(c, i, data);
//    }
//
//    AVLNode* min = c.root;
//    while (min->left) 
//    {
//        min = min->left;
//    }
//    for (uint32_t i = 0; i < sz; ++i) 
//    {
//        AVLNode* node = data->tree.Avlt_Offset(min, (int64_t)i);
//        assert(CONTAINER_OF(node, Data, node)->val == i);
//
//        for (uint32_t j = 0; j < sz; ++j) 
//        {
//            int64_t offset = (int64_t)j - (int64_t)i;
//            AVLNode* n2 = data->tree.Avlt_Offset(node, offset);
//            assert(CONTAINER_OF(n2, Data, node)->val == j);
//        }
//        assert(!data->tree.Avlt_Offset(node, -(int64_t)i - 1));
//        assert(!data->tree.Avlt_Offset(node, sz - i));
//    }
//
//    dispose(c.root);
//}
//
//int main() 
//{
//    for (uint32_t i = 1; i <= 500; ++i) {
//        test_case(i);
//    }
//    return 0;
//}

//----------------------------
// Test case result? : Passed
//----------------------------