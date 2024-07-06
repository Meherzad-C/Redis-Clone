#pragma once
// Circular Doubly Linked List
class DList 
{
public:
    DList* prev;
    DList* next;

public:
    DList();
    ~DList();

public:
    bool Empty() const;
    void Detach();
    void Insert_Before(DList* rookie);
};