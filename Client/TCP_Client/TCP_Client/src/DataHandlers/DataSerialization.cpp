#include "DataSerialization.h"

DataSerializer::DataSerializer()
{
}

DataSerializer::~DataSerializer()
{
}

int32_t DataSerializer::OnResponse(const uint8_t* data, size_t size) {
    if (size < 1) {
        Msg("bad response");
        return -1;
    }
    switch (data[0]) 
    {
        case SERIALIZATION_NIL:
            printf("(nil)\n");
            return 1;
        case SERIALIZATION_ERROR:
            return HandleError(data, size);
        case SERIALIZATION_STRING:
            return HandleString(data, size);
        case SERIALIZATION_INT:
            return HandleInt(data, size);
        case SERIALIZATION_DOUBLE:
            return HandleDouble(data, size);
        case SERIALIZATION_ARRAY:
            return HandleArray(data, size);
        default:
            Msg("bad response");
        return -1;
    }
}

int32_t DataSerializer::HandleError(const uint8_t* data, size_t size) 
{
    if (size < 1 + 8) 
    {
        Msg("bad response");
        return -1;
    }
    int32_t code = 0;
    uint32_t len = 0;
    memcpy(&code, &data[1], 4);
    memcpy(&len, &data[1 + 4], 4);
    if (size < 1 + 8 + len) 
    {
        Msg("bad response");
        return -1;
    }
    printf("(err) %d %.*s\n", code, len, &data[1 + 8]);
    return 1 + 8 + len;
}

int32_t DataSerializer::HandleString(const uint8_t* data, size_t size) 
{
    if (size < 1 + 4) 
    {
        Msg("bad response");
        return -1;
    }
    uint32_t len = 0;
    memcpy(&len, &data[1], 4);
    if (size < 1 + 4 + len) 
    {
        Msg("bad response");
        return -1;
    }
    printf("(str) %.*s\n", len, &data[1 + 4]);
    return 1 + 4 + len;
}

int32_t DataSerializer::HandleInt(const uint8_t* data, size_t size) 
{
    if (size < 1 + 8) {
        Msg("bad response");
        return -1;
    }
    int64_t val = 0;
    memcpy(&val, &data[1], 8);
    printf("(int) %ld\n", val);
    return 1 + 8;
}

int32_t DataSerializer::HandleDouble(const uint8_t* data, size_t size)
{
    if (size < 1 + 8) 
    {
        Msg("bad response");
        return -1;
    }

    double val = 0;
    memcpy(&val, &data[1], 8);
    printf("(dbl) %g\n", val);
    return 1 + 8;
}

int32_t DataSerializer::HandleArray(const uint8_t* data, size_t size) 
{
    if (size < 1 + 4) 
    {
        Msg("bad response");
        return -1;
    }
    uint32_t len = 0;
    memcpy(&len, &data[1], 4);
    printf("(arr) len=%u\n", len);
    size_t arr_bytes = 1 + 4;
    for (uint32_t i = 0; i < len; ++i) 
    {
        int32_t rv = OnResponse(&data[arr_bytes], size - arr_bytes);
        if (rv < 0) 
        {
            return rv;
        }
        arr_bytes += (size_t)rv;
    }
    printf("(arr) end\n");
    return (int32_t)arr_bytes;
}
