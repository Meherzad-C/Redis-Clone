//#include <iostream>
//#include <set>
//#include "..\Src\Data_Structures\AVLTree\AVLTree.h"
//
//void TestInsert(uint32_t sz) 
//{
//    for (uint32_t val = 0; val < sz; ++val) 
//    {
//        AVLTree tree;
//        std::multiset<uint32_t> ref;
//        for (uint32_t i = 0; i < sz; ++i) 
//        {
//            if (i == val) 
//            {
//                continue;
//            }
//            tree.Add(i);
//            ref.insert(i);
//        }
//        tree.Verify(ref);
//
//        tree.Add(val);
//        ref.insert(val);
//        tree.Verify(ref);
//        tree.Dispose();
//    }
//}
//
//void TestInsertDuplicate(uint32_t sz) 
//{
//    for (uint32_t val = 0; val < sz; ++val) 
//    {
//        AVLTree tree;
//        std::multiset<uint32_t> ref;
//        for (uint32_t i = 0; i < sz; ++i) 
//        {
//            tree.Add(i);
//            ref.insert(i);
//        }
//        tree.Verify(ref);
//
//        tree.Add(val);
//        ref.insert(val);
//        tree.Verify(ref);
//        tree.Dispose();
//    }
//}
//
//void TestRemove(uint32_t sz) 
//{
//    for (uint32_t val = 0; val < sz; ++val) 
//    {
//        AVLTree tree;
//        std::multiset<uint32_t> ref;
//        for (uint32_t i = 0; i < sz; ++i) 
//        {
//            tree.Add(i);
//            ref.insert(i);
//        }
//        tree.Verify(ref);
//
//        assert(tree.Del(val));
//        ref.erase(val);
//        tree.Verify(ref);
//        tree.Dispose();
//    }
//}
//
//int main() 
//{
//    AVLTree tree;
//
//    // some quick tests
//    tree.Verify({});
//    tree.Add(123);
//    tree.Verify({ 123 });
//    assert(!tree.Del(124));
//    assert(tree.Del(123));
//    tree.Verify({});
//
//    // sequential insertion
//    std::multiset<uint32_t> ref;
//    for (uint32_t i = 0; i < 1000; i += 3) 
//    {
//        tree.Add(i);
//        ref.insert(i);
//        tree.Verify(ref);
//    }
//
//    // random insertion
//    for (uint32_t i = 0; i < 100; i++) 
//    {
//        uint32_t val = (uint32_t)rand() % 1000;
//        tree.Add(val);
//        ref.insert(val);
//        tree.Verify(ref);
//    }
//
//    // random deletion
//    for (uint32_t i = 0; i < 200; i++) 
//    {
//        uint32_t val = (uint32_t)rand() % 1000;
//        auto it = ref.find(val);
//        if (it == ref.end()) 
//        {
//            assert(!tree.Del(val));
//        }
//        else 
//        {
//            assert(tree.Del(val));
//            ref.erase(it);
//        }
//        tree.Verify(ref);
//    }
//
//    // insertion/deletion at various positions
//    for (uint32_t i = 0; i < 200; ++i) 
//    {
//        TestInsert(i);
//        TestInsertDuplicate(i);
//        TestRemove(i);
//    }
//
//    tree.Dispose();
//    return 0;
//}

//----------------------------
// Test case result? : Passed
//----------------------------