#pragma once
#include <cstdint>

// Get the address of the containing structure from a pointer to one of its members
#define container_of(ptr, type, member) \
    (reinterpret_cast<type*>( \
        reinterpret_cast<char*>(ptr) - offsetof(type, member) \
    ))

inline uint64_t StrHash(const uint8_t* data, size_t len)
{
	uint32_t h = 0x811C9DC5;
	for (size_t i = 0; i < len; i++) 
	{
		h = (h + data[i]) * 0x01000193;
	}
	return h;
}