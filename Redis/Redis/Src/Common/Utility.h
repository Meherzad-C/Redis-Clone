#pragma once
#include <cstdint>
#include <string>

// ****************************************************
// Common Utility functions are thrown in this file
// ****************************************************

// Get the address of the containing structure 
// from a pointer to one of its members
#define CONTAINER_OF(ptr, type, member) \
    (reinterpret_cast<type*>( \
        reinterpret_cast<char*>(ptr) - offsetof(type, member) \
    ))

inline uint64_t StringHash(const uint8_t* data, size_t len)
{
	uint32_t h = 0x811C9DC5;
	for (size_t i = 0; i < len; i++) 
	{
		h = (h + data[i]) * 0x01000193;
	}
	return h;
}

inline uint32_t _Min(size_t lhs, size_t rhs) {

	return lhs < rhs ? lhs : rhs;
}

inline uint32_t _Max(uint32_t lhs, uint32_t rhs)
{
	return lhs < rhs ? rhs : lhs;
}

inline bool StringToInt(const std::string& s, int64_t& out) 
{
	char* endp = NULL;
	out = strtoll(s.c_str(), &endp, 10);
	return endp == s.c_str() + s.size();
}

inline bool StringToDouble(const std::string& s, double& out)
{
	char* endp = NULL;
	out = strtod(s.c_str(), &endp);
	return endp == s.c_str() + s.size() && !isnan(out);
}

template <typename T>
inline bool PointerEqual(T* lhs, T* rhs)
{
	return lhs == rhs;
}