#pragma once

#include <stddef.h>
#include <stdint.h>
#include <vector>
#include <stdexcept>

// project includes
#include "HeapItem.h"

// Binary Min-Heap
class Heap 
{
private:
    size_t Parent(size_t i) const;

    void HeapUp(size_t pos);
    void HeapDown(size_t pos);

public:
    std::vector<HeapItem> data;

public:
    Heap() = default;
    ~Heap() = default;

    size_t Left(size_t i) const;
    size_t Right(size_t i) const;
    void Update(size_t pos);

    // helpers
    void Push(HeapItem item);
    void Pop();
    size_t Size() const;
    HeapItem& Front();
    const HeapItem& Front() const;
    HeapItem& Back();
    const HeapItem& Back() const;
    HeapItem& AccessElement(size_t pos);
    const HeapItem& AccessElement(size_t pos) const;
    bool Empty() const;
};