#ifndef DATA_H_
#define DATA_H_

#include "selene.h"
#include "lua_helper.h"

typedef struct Data Data;

META_FUNCTION(Data, Realloc);
META_FUNCTION(Data, Free);
META_FUNCTION(Data, GetSize);
META_FUNCTION(Data, GetPointer);
META_FUNCTION(Data, GetString);
META_FUNCTION(Data, ReadBytes);
META_FUNCTION(Data, ReadInts);
META_FUNCTION(Data, ReadFloats);
META_FUNCTION(Data, WriteByte);
META_FUNCTION(Data, WriteInt);
META_FUNCTION(Data, WriteFloat);
META_FUNCTION(Data, WriteString);

#endif // DATA_H_
