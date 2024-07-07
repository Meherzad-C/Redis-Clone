#include "Heap.h"

size_t Heap::Parent(size_t i) const 
{
    return (i + 1) / 2 - 1;
}

size_t Heap::Left(size_t i) const 
{
    return i * 2 + 1;
}

size_t Heap::Right(size_t i) const 
{
    return i * 2 + 2;
}

void Heap::Update(size_t pos) 
{
    if (pos > 0 && data[Parent(pos)].val > data[pos].val) 
    {
        HeapUp(pos);
    }
    else 
    {
        HeapDown(pos);
    }
}

void Heap::HeapUp(size_t pos) 
{
    HeapItem t = data[pos];
    while (pos > 0 && data[Parent(pos)].val > t.val) 
    {
        // swap with the parent
        data[pos] = data[Parent(pos)];
        *data[pos].ref = pos;
        pos = Parent(pos);
    }
    data[pos] = t;
    *data[pos].ref = pos;
}

void Heap::HeapDown(size_t pos) 
{
    size_t len = data.size();
    HeapItem t = data[pos];
    while (true) 
    {
        // find the smallest one among the parent and their kids
        size_t l = Left(pos);
        size_t r = Right(pos);
        size_t min_pos = pos;
        size_t min_val = t.val;
        if (l < len && data[l].val < min_val) 
        {
            min_pos = l;
            min_val = data[l].val;
        }
        if (r < len && data[r].val < min_val) 
        {
            min_pos = r;
        }
        if (min_pos == pos) 
        {
            break;
        }
        // swap with the kid
        data[pos] = data[min_pos];
        *data[pos].ref = pos;
        pos = min_pos;
    }
    data[pos] = t;
    *data[pos].ref = pos;
}

void Heap::Push(HeapItem item) 
{
    data.push_back(item);
    HeapUp(data.size() - 1);
}

HeapItem Heap::Pop() 
{
    if (data.empty()) 
    {
        return HeapItem(); // or throw an exception
    }
    HeapItem top = data.front();
    data[0] = data.back();
    data.pop_back();
    if (!data.empty()) 
    {
        HeapDown(0);
    }
    return top;
}

size_t Heap::Size() const 
{
    return data.size();
}