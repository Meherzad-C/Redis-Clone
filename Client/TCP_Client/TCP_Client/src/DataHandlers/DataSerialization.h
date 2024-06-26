#pragma once

#include <cstdint>
#include <cstring>
#include <cstdio>

#include "../Common/Utility.h"

class DataSerializer 
{
private:
    static int32_t HandleError(const uint8_t* data, size_t size);
    static int32_t HandleString(const uint8_t* data, size_t size);
    static int32_t HandleInt(const uint8_t* data, size_t size);
    static int32_t HandleDouble(const uint8_t* data, size_t size);
    static int32_t HandleArray(const uint8_t* data, size_t size);

public:
    enum Type 
    {
        SERIALIZATION_NIL = 0,
        SERIALIZATION_ERROR = 1,
        SERIALIZATION_STRING = 2,
        SERIALIZATION_INT = 3,
        SERIALIZATION_DOUBLE = 4,
        SERIALIZATION_ARRAY = 5,
    };

    static int32_t OnResponse(const uint8_t* data, size_t size);

    DataSerializer();
    ~DataSerializer();
};
