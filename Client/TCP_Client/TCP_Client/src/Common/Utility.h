#pragma once

#include <stdio.h>

inline void Msg(const char* msg)
{
	fprintf(stderr, "%s\n", msg);
}