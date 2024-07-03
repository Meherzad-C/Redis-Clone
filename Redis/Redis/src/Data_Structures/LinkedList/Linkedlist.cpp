#include "Linkedlist.h"

DList::DList() : prev(this), next(this)
{
}

bool DList::Empty() const
{
	return this->next == this;
}

void DList::Detach()
{
    DList* prev = this->prev;
    DList* next = this->next;
    prev->next = next;
    next->prev = prev;
}

void DList::Insert_Before(DList* rookie)
{
    DList* prev = this->prev;
    prev->next = rookie;
    rookie->prev = prev;
    rookie->next = this;
    this->prev = rookie;
}

DList::~DList()
{
}