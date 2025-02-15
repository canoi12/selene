#ifndef LUA_HELPER_H_
#define LUA_HELPER_H_

#define LUA_STATE_NAME L

#define LUA_META_CLASS(x) #x

#define INIT_ARG()\
int arg = 1

#define PUSH_VALUE(index)\
lua_pushvalue(LUA_STATE_NAME, index)

#define PUSH_NIL()\
lua_pushnil(LUA_STATE_NAME)

#define MODULE_FUNCTION(name, func)\
int l_##name##_##func(lua_State* LUA_STATE_NAME)

#define BEGIN_MODULE(name)\
int luaopen_##name(lua_State* LUA_STATE_NAME)

#define LOAD_MODULE(name)\
luaopen_##name(LUA_STATE_NAME);\
lua_setfield(LUA_STATE_NAME, -2, #name)

#define PUSH_CFUNC(func)\
lua_pushcfunction(LUA_STATE_NAME, func)

#define SET_FIELD(i, f)\
lua_setfield(LUA_STATE_NAME, i, f)

// LuaReg
#define BEGIN_REG(name)\
luaL_Reg name[] = {

#define END_REG()\
    {NULL, NULL}\
};

#define REG_FIELD(type, name)\
{#name, l_##type##_##name}

#define REG_META_FIELD(type, name)\
{#name, l_##type##__##name}

// MetaReg

#define BEGIN_META_REG(...)\
lua_FieldReg __VA_ARGS__[] = {

#define END_META_REG()\
{NULL, NULL, NULL}\
};

#define META_REG_FIELD(type, name)\
{#name, l_##type##_get_##name, l_##type##_set_##name}

#define META_REG_GET_FIELD(type, name)\
{#name, l_##type##_get_##name, NULL}

#define META_REG_SET_FIELD(type, name)\
{#name, NULL, l_##type##_set_##name}

// Enum
#define BEGIN_ENUM(enum_name)\
struct { const char* name; int value; } enum_name[] = {

#define END_ENUM()\
    {NULL, 0},\
};

#define ENUM_FIELD(name, ...)\
{#name, __VA_ARGS__##name}

#define LOAD_ENUM(enum_name)\
for (int i = 0; enum_name[i].name != NULL; i++) {\
    lua_pushinteger(LUA_STATE_NAME, enum_name[i].value);\
    lua_setfield(LUA_STATE_NAME, -2, enum_name[i].name);\
}

// Meta
#define CHECK_META(type)\
INIT_ARG();\
CHECK_UDATA(type, self)

#define GET_META(type)\
INIT_ARG();\
GET_UDATA(type, self)

#define META_FUNCTION(type, func)\
int l_##type##__##func(lua_State* LUA_STATE_NAME)

#define LOAD_META(type)\
l_##type##_meta(LUA_STATE_NAME);\
lua_setfield(LUA_STATE_NAME, -2, #type)

#define BEGIN_META(type)\
int l_##type##_meta(lua_State* L)

#define NEW_META(type, lib, index)\
luaL_newmetatable(LUA_STATE_NAME, #type);\
luaL_setfuncs(LUA_STATE_NAME, lib, 0);\
luaL_newlib(LUA_STATE_NAME, index);\
lua_setfield(LUA_STATE_NAME, -2, "__index")

// User Data

#define NEW_UDATA(type, name)\
type* name = (type*)lua_newuserdata(LUA_STATE_NAME, sizeof(type));\
luaL_setmetatable(LUA_STATE_NAME, #type)

#define NEW_UDATA_ADD(type, name, size)\
type* name = (type*)lua_newuserdata(LUA_STATE_NAME, sizeof(type) + (size));\
luaL_setmetatable(LUA_STATE_NAME, #type)

#define GET_UDATA(type, name)\
type* name = (type*)lua_touserdata(LUA_STATE_NAME, arg++)

#define CHECK_UDATA(type, name)\
type* name = (type*)luaL_checkudata(LUA_STATE_NAME, arg++, #type)

#define TEST_UDATA(type, name)\
type* name = (type*)luaL_testudata(LUA_STATE_NAME, arg++, #type)

// Light User Data

#define GET_LUDATA(type, name)\
type* name = (type*)lua_touserdata(LUA_STATE_NAME, arg++)

#define CHECK_LUDATA(type, name)\
type* name = NULL;\
if (lua_type(LUA_STATE_NAME, arg) != LUA_TLIGHTUSERDATA) return luaL_error(LUA_STATE_NAME, "Invalid argument, lightuserdata expected");\
else name = (type*)lua_touserdata(LUA_STATE_NAME, arg++)

#define PUSH_LUDATA(name)\
lua_pushlightuserdata(LUA_STATE_NAME, name)

// Integer

#define GET_INTEGER(name)\
int name = (int)lua_tointeger(LUA_STATE_NAME, arg++)

#define CHECK_INTEGER(name)\
int name = (int)luaL_checkinteger(LUA_STATE_NAME, arg++)

#define OPT_INTEGER(name, def)\
int name = (int)luaL_optinteger(LUA_STATE_NAME, arg++, def)

#define PUSH_INTEGER(name)\
lua_pushinteger(LUA_STATE_NAME, name)

// Number

#define GET_NUMBER(type, name)\
type name = lua_tonumber(LUA_STATE_NAME, arg++)

#define CHECK_NUMBER(type, name)\
type name = (type)luaL_checknumber(LUA_STATE_NAME, arg++)

#define OPT_NUMBER(type, name, def)\
type name = (type)luaL_optnumber(LUA_STATE_NAME, arg++, def)

#define PUSH_NUMBER(name)\
lua_pushnumber(LUA_STATE_NAME, name)

// String

#define GET_STRING(name)\
const char* name = lua_tostring(LUA_STATE_NAME, arg++)

#define CHECK_STRING(name)\
const char* name = luaL_checkstring(LUA_STATE_NAME, arg++)

#define CHECK_LSTRING(name, len)\
const char* name = luaL_checklstring(LUA_STATE_NAME, arg++, len)

#define PUSH_STRING(name)\
lua_pushstring(LUA_STATE_NAME, name)

// Boolean

#define GET_BOOLEAN(name)\
int name = (int)lua_toboolean(L, arg++)

#define CHECK_BOOLEAN(name)\
int name;\
if (lua_type(L, arg) == LUA_TBOOLEAN)\
    name = lua_toboolean(LUA_STATE_NAME, arg++);\
else\
    return luaL_argerror(LUA_STATE_NAME, arg, "Boolean expected")

#define PUSH_BOOLEAN(name)\
lua_pushboolean(LUA_STATE_NAME, name)

// Binds for Struct Fields


// Integer

#define META_INT_GET_FIELD(type, field)\
static int l_##type##_get_##field(lua_State* LUA_STATE_NAME) {\
    CHECK_META(type);\
    PUSH_INTEGER(self->field);\
    return 1;\
}

#define META_INT_SET_FIELD(type, field)\
static int l_##type##_set_##field(lua_State* LUA_STATE_NAME) {\
    CHECK_META(type);\
    CHECK_INTEGER(value);\
    self->field = value;\
    return 0;\
}

#define META_INT_FIELD(type, field)\
META_INT_GET_FIELD(type, field)\
META_INT_SET_FIELD(type, field)

// Number

#define META_NUM_GET_FIELD(type, num_type, field)\
static int l_##type##_get_##field(lua_State* L) {\
    CHECK_META(type);\
    PUSH_NUMBER(num_type, self->field);\
    return 1;\
}

#define META_NUM_SET_FIELD(type, num_type, field)\
static int l_##type##_set_##field(lua_State* L) {\
    CHECK_META(type);\
    CHECK_NUMBER(num_type, value);\
    self->field = value;\
    return 0;\
}

#define META_NUM_FIELD(type, num_type, field)\
META_NUM_GET_FIELD(type, num_type, field)\
META_NUM_SET_FIELD(type, num_type, field)

// Light User Data

#define META_PTR_GET_FIELD(type, ptr_type, field)\
static int l_##type##_get_##field(lua_State* L) {\
    CHECK_META(type);\
    PUSH_LUDATA(self->field);\
    return 1;\
}

#define META_PTR_SET_FIELD(type, ptr_type, field)\
static int l_##type##_set_##field(lua_State* L) {\
    CHECK_META(type);\
    CHECK_LUDATA(ptr_type, value);\
    self->field = value;\
    return 0;\
}

#define META_PTR_FIELD(type, ptr_type, field)\
META_PTR_GET_FIELD(type, ptr_type, field)\
META_PTR_SET_FIELD(type, ptr_type, field)

// String

#define META_STR_GET_FIELD(type, field)\
static int l_##type##_get_##field(lua_State* L) {\
    CHECK_META(type);\
    PUSH_STRING(self->field);\
    return 1;\
}

#define META_STR_FIELD(type, field)\
static int l_##type##_get_##field(lua_State* L) {\
    CHECK_META(type);\
    PUSH_STRING(self->field);\
    return 1;\
}/*\
static int l_##type##_set_##field(lua_State* L) {\
    CHECK_META(type);\
    CHECK_STRING(value);\
    self->##size = value;\
    return 0;\
}*/

#define LOAD_META_FIELDS(type, reg_name)\
lua_newtable(L);\
for (int i = 0; (reg_name)[i].name != NULL; i++) {\
    if ((reg_name)[i].get)\
        lua_pushcfunction(L, (reg_name)[i].get);\
    else\
        lua_pushnil(L);\
    lua_setfield(L, -2, (reg_name)[i].name);\
}\
lua_rawsetp(LUA_STATE_NAME, LUA_REGISTRYINDEX, l_##type##__index);\
lua_newtable(L);\
for (int i = 0; (reg_name)[i].name != NULL; i++) {\
    if ((reg_name)[i].set)\
        lua_pushcfunction(L, (reg_name)[i].set);\
    else\
        lua_pushnil(L);\
    lua_setfield(L, -2, (reg_name)[i].name);\
}\
lua_rawsetp(LUA_STATE_NAME, LUA_REGISTRYINDEX, l_##type##__newindex)

#define LOAD_GET_META_FIELDS(type, reg_name)\
lua_newtable(L);\
for (int i = 0; (reg_name)[i].name != NULL; i++) {\
    if ((reg_name)[i].get)\
        lua_pushcfunction(L, (reg_name)[i].get);\
    else\
        lua_pushnil(L);\
    lua_setfield(L, -2, (reg_name)[i].name);\
}\
lua_rawsetp(LUA_STATE_NAME, LUA_REGISTRYINDEX, l_##type##__index)

typedef struct {
    const char* name;
    lua_CFunction get;
    lua_CFunction set;
} lua_FieldReg;

#endif /* LUA_HELPER_H_ */
