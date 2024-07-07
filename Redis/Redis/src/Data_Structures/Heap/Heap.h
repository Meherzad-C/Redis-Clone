#pragma once

#include <stddef.h>
#include <stdint.h>
#include <vector>

// project includes
#include "HeapItem.h"

class Heap 
{
private:
    size_t Parent(size_t i) const;
    size_t Left(size_t i) const;
    size_t Right(size_t i) const;

    void HeapUp(size_t pos);
    void HeapDown(size_t pos);

public:
    std::vector<HeapItem> data;

public:
    Heap() = default;
    ~Heap() = default;

    void Update(size_t pos);
    void Push(HeapItem item);
    HeapItem Pop();
    size_t Size() const;

};