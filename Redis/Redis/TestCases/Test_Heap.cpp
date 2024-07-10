//#include <assert.h>
//#include <vector>
//#include <map>
//#include <basetsd.h>
//#include "../Src/Data_Structures/Heap/Heap.h"
//
//
//struct Data 
//{
//    size_t heapIdx = -1;
//};
//
//struct Container 
//{
//    Heap heap;
//    std::multimap<uint64_t, Data*> map;
//};
//
//static void Dispose(Container& c) 
//{
//    for (auto p : c.map) 
//    {
//        delete p.second;
//    }
//}
//
//static void Add(Container& c, uint64_t val) 
//{
//    Data* d = new Data();
//    c.map.insert(std::make_pair(val, d));
//    HeapItem item;
//    item.ref = &d->heapIdx;
//    item.val = val;
//    c.heap.Push(item);
//    c.heap.Update(c.heap.Size() - 1);
//}
//
//static void Del(Container& c, uint64_t val) 
//{
//    auto it = c.map.find(val);
//    assert(it != c.map.end());
//    Data* d = it->second;
//    assert(c.heap.AccessElement(d->heapIdx).val == val);
//    assert(c.heap.AccessElement(d->heapIdx).ref == &d->heapIdx);
//    c.heap.AccessElement(d->heapIdx) = c.heap.Back();
//    c.heap.Pop();
//    if (d->heapIdx < c.heap.Size()) 
//    {
//        c.heap.Update(d->heapIdx);
//    }
//    delete d;
//    c.map.erase(it);
//}
//
//static void Verify(Container& c) 
//{
//    assert(c.heap.Size() == c.map.size());
//    for (size_t i = 0; i < c.heap.Size(); ++i) 
//    {
//        size_t l = c.heap.Left(i);
//        size_t r = c.heap.Right(i);
//        assert(l >= c.heap.Size() || c.heap.AccessElement(l).val >= c.heap.AccessElement(i).val);
//        assert(r >= c.heap.Size() || c.heap.AccessElement(r).val >= c.heap.AccessElement(i).val);
//        assert(*c.heap.AccessElement(i).ref == i); 
//    }
//}
//
//static void TestCase(size_t sz) 
//{
//    for (uint32_t j = 0; j < 2 + sz * 2; ++j) 
//    {
//        Container c;
//        for (uint32_t i = 0; i < sz; ++i) 
//        {
//            Add(c, 1 + i * 2);
//        }
//        Verify(c);
//
//        Add(c, j);
//        Verify(c);
//
//        Dispose(c);
//    }
//
//    for (uint32_t j = 0; j < sz; ++j) 
//    {
//        Container c;
//        for (uint32_t i = 0; i < sz; ++i) 
//        {
//            Add(c, i);
//        }
//        Verify(c);
//
//        Del(c, j);
//        Verify(c);
//
//        Dispose(c);
//    }
//}
//
//int main() 
//{
//    for (uint32_t i = 0; i < 200; ++i) 
//    {
//        TestCase(i);
//    }
//    return 0;
//}

//----------------------------
// Test case result? : Passed
//----------------------------